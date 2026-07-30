#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include "display_macros.h"
#include "buzzer.h"
#include "timer.h"
#include "display.h"
#include "initialisation.h"
#include "lfsr.h"
#include "adc.h"
#include "uart.h"

typedef enum{
    SIMON_GENERATE,
    SIMON_PLAYON, // SIMON buzzer & display on
    SIMON_PLAYOFF, // SIMON buzzer & display off
    AWAITING_INPUT,
    HANDLE_INPUT,
    EVALUATE_INPUT,
    SUCC, 
    FAIL,
    DISP_SCORE,
    DISP_BLANK
} GAME_state;

GAME_state state = SIMON_GENERATE; // initial state

uint8_t step = 0; // EXPECTED curr step
uint8_t player_input = 255; // QUTy input from player

uint16_t sequence_len = 1;
uint16_t sequence_index = 0; // input index counter from player
uint16_t score = 1; // init score

uint16_t PLAYBACK_delay = 250;
uint16_t half_delay = 125;

//uint8_t pb_state = 0xFF;
//uint8_t pb_state_r = 0xFF;
//uint8_t pb_changed = 0;

uint8_t input_from_uart = 0; // uart input from player

// new seed from uart input initialise
uint8_t seed_collect = 0; // collecting plyer input for 8 digits of hex
uint8_t seed_digits = 0; // collected digits by now
uint8_t seed_invalid = 0; // invalid digits collected
uint32_t seed_val = 0; // the new hex seed value
uint32_t pending_seed = LFSR_SEED; // collected down and update new seed
uint8_t pending_seed_valid = 0; // check seed validity

void state_machine(void);

static void apply_pending_seed (void) {
    if (pending_seed_valid) {
        set_lfsr_seed(pending_seed);
        pending_seed_valid = 0;
    }
}

// convert uart 'char' input to 'int' e.g.: '9' -> 9
static uint8_t hex_val(char c, uint8_t *val) {
    if (c >= '0' && c <= '9') {
        *val = (uint8_t)(c - '0');
        return 1;
    } if (c >= 'a' && c <= 'f') {
        *val = (uint8_t)(c - 'a' + 10);
        return 1;
    } if (c >= 'A' && c <= 'F') {
        *val = (uint8_t)(c - 'A' + 10);
        return 1;
    }
    return 0;
}

// handle seed uart input & create these input as seed
static void handle_sed_char(char c) {
    uint8_t digit = 0;

    if (!hex_val(c, &digit)) {
        seed_invalid = 1;
        digit = 0;
    }

    // shift left the original 4 bit seed
    seed_val = (seed_val << 4) | digit;
    seed_digits++; // continue increment to keep moving shift left
    
    // if collect all 8 digits && no invalid char
    // save these digits to new seed
    if (seed_digits == 8) {
        if(!seed_invalid) {
            pending_seed = seed_val;
            pending_seed_valid = 1;
        }
        seed_collect = 0;
        seed_digits = 0;
        seed_invalid = 0;
        seed_val = 0;
    }
}

int main(void)
{
    cli(); // disable interrupt
    // call the initialisation functions
    initial_timer();
    initial_uart();
    initial_display();
    initial_buzzer();
    initial_adc();
    initial_pb();
    init_lfsr();
   

    sei(); // re-enable interrupt

    state_machine();

    // The program should not reach this point
    while (1)
        ;
}

// this function used for handle input
// 'button' replaced by 'player input' from handle
// return '1' as released due to active low, '1' == True
// this func only excutes when putton released(returns True)
static uint8_t button_released(uint8_t button) {
    switch (button)
    {
    case 0:
        return (pb_debounced & PIN4_bm); // s1 released
    case 1:
        return (pb_debounced & PIN5_bm); // s2 released
    case 2:
        return (pb_debounced & PIN6_bm); //s3 released
    case 3:
        return (pb_debounced & PIN7_bm); // s4 released
    default:
        return 1;
    }
}

static void reset_game(void) {
    // off buzzer & display when reset
    stop_tone();
    display_off();

    // back to the initial seed
    apply_pending_seed();
    init_lfsr();

    octave_reset(); // reset octave freq
    // index set back to 0
    sequence_len = 1;
    sequence_index = 0;
    score = 1;

    // clear current player inputting status
    player_input = 255;
    input_from_uart = 0;
    seed_collect = 0;
    seed_digits = 0;
    seed_invalid = 0;
    seed_val = 0;

    pb_falling = 0x00;

    elapsed_t = 0;
    state = SIMON_GENERATE;
}

static void handle_uart(void) {
    uint8_t uart_input = 255;

    while (uart_available()) {
        if (seed_collect) {
            handle_sed_char(uart_getc());
            continue;
        }

        uartcommand cmd = uart_get_command(&uart_input);

        if (cmd == UART_RESET) {
            reset_game();
            return;
        }
        else if (cmd == UART_INC_FREQ) {
            incre_octave();
            return;
        }
        else if (cmd == UART_DEC_FREQ) {
            decre_octave();
            return;
        }
        else if (cmd == UART_SEED) {
            seed_collect = 1;
            seed_digits = 0;
            seed_invalid = 0;
            seed_val = 0;
            return;
        }
        else if (cmd == UART_GAME_COMMAND) {
            if (state == AWAITING_INPUT) {
                player_input = uart_input;
                input_from_uart = 1;

                update_playback_delay();

                elapsed_t = 0;
                tone_select(player_input);
                display_step(player_input);
                state = HANDLE_INPUT;
            }
            return;
        }
    }

}


void state_machine(void)
{
    // initialisation
    while (1)
    {
        // global uart reset, reset/interrupt current disp,sequence at any time
        handle_uart();

        //pb_state_r = pb_state;
        //pb_state = pb_debounced;
        //pb_changed = pb_state_r ^ pb_state; // check 'changed' bit with XOR
        //pb_falling = pb_changed & pb_state_r; // 1 -> 0

        switch (state) {
            case SIMON_GENERATE:
            if (sequence_index == sequence_len) {
                save_lfsr(); // save the position
                reset_lfsr(); // round to the start
                sequence_index = 0;
                step = next_lfsr(); // prepare 1st expected input step from player
                state = AWAITING_INPUT;
            } else {
                step = next_lfsr(); // update step
                sequence_index++;

                // update delay while generate disp
                update_playback_delay(); 

                tone_select(step);
                display_step(step);

                elapsed_t = 0;
                state = SIMON_PLAYON;
            }
            break;

            case SIMON_PLAYON: 
            if (elapsed_t >= half_delay) {
                stop_tone();
                display_off();
                elapsed_t = 0;
                state = SIMON_PLAYOFF;
            }
            break;

            case SIMON_PLAYOFF:
            if (elapsed_t >= half_delay) {
                elapsed_t = 0;
                state = SIMON_GENERATE;
            }
            break;

            case AWAITING_INPUT: {

                if (pb_falling & PIN4_bm) {
                
                // mark the falling edge has been recorded
                // incase duplicated read the same falling status
                pb_falling &= ~PIN4_bm; // clear S1 when pressed
                
                player_input = 0;
                input_from_uart = 0;
                // update delay while player inputting
                update_playback_delay();
                elapsed_t = 0;
                tone_select(player_input);
                display_step(player_input);
                state = HANDLE_INPUT;
            } else if (pb_falling & PIN5_bm) {
                pb_falling &= ~PIN5_bm;
                player_input = 1;
                input_from_uart = 0;
                update_playback_delay();
                elapsed_t = 0;
                tone_select(player_input);
                display_step(player_input);
                state = HANDLE_INPUT;
            } else if (pb_falling & PIN6_bm) {
                pb_falling &= ~PIN6_bm;
                player_input = 2;
                input_from_uart = 0;
                update_playback_delay();
                elapsed_t = 0;
                tone_select(player_input);
                display_step(player_input);
                state = HANDLE_INPUT;
            } else if (pb_falling & PIN7_bm) {
                pb_falling &= ~PIN7_bm;
                player_input = 3;
                input_from_uart = 0;
                update_playback_delay();
                elapsed_t = 0;
                tone_select(player_input);
                display_step(player_input);
                state = HANDLE_INPUT;
            }
            break;
            }

            // handle when turn disp & buzz off -- QUTy
            // conditions: time exceed half delay && player release the button -- QUTy
            // only check half delay reached or not for keyboard
            case HANDLE_INPUT:
            if (elapsed_t >= half_delay) {
                if (input_from_uart || button_released(player_input)) {
                stop_tone();
                display_off();
                elapsed_t = 0;
                state = EVALUATE_INPUT;
                }   
            }
            break;

            case EVALUATE_INPUT:
            // if step != pb: fail
            // if step == pb && sequence_index < sequence_len:
            // nxt_lfsr, index++, awaiting input (player not finish yet)
            // if step == pb && sequence_index < sequence_len: succ
            if (player_input != step) 
            {
                display_fail();
                elapsed_t = 0;

                score = sequence_len;

                uart_puts("GAME OVER\n");
                uart_put_uint16(score);
                uart_putc('\n');
                
                state = FAIL;
            } 
            else 
            {
                sequence_index++; 

                if (sequence_index == sequence_len) 
                {
                    display_success();
                    elapsed_t = 0;

                    score = sequence_len;

                    uart_puts("SUCCESS\n");
                    uart_put_uint16(score);
                    uart_putc('\n');

                    reset_lfsr();
                    sequence_index = 0;
                    sequence_len++;
                    apply_pending_seed();

                    state = SUCC;
                } 
                else // for this else, player input correct
                     // but not finishing input
                {
                    step = next_lfsr(); // prepare nxt expected input step
                    state = AWAITING_INPUT;
                }
            }
            break;    

            case SUCC:
            if (elapsed_t >= PLAYBACK_delay) {
                display_off();
                elapsed_t = 0;

                state = SIMON_GENERATE;
            }
            break;

            case FAIL:
            if (elapsed_t >= PLAYBACK_delay) {
                display_score(score);
                elapsed_t = 0;

                state = DISP_SCORE;
            }
            break;

            case DISP_SCORE:
            if (elapsed_t >= PLAYBACK_delay) {
                display_off();
                elapsed_t = 0;
                
                state = DISP_BLANK;
            }
            break;

            // BLANK shows after fail
            case DISP_BLANK:
            if (elapsed_t >= PLAYBACK_delay) {
                if(pending_seed_valid) {
                    apply_pending_seed();
                } else {
                    advanced_lfsr(); // next around after fail disp_score
                }

                sequence_len = 1; // reset the sequence to 1 step for new try
                sequence_index = 0; // clear current index and new try index from 0
                score = 1; // clear score from last game

                elapsed_t = 0;
                state = SIMON_GENERATE;
            }
            break;

            default:
            state = SIMON_GENERATE;
            break;
        }
    }
}
