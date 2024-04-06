/*
 * File:   main.c
 * Author: HP
 *
 * Created on 17 March, 2024, 5:19 PM
 */


#include <xc.h>
#include "main.h" /* Declaring all header file */
#include "digital_keypad.h"
#include "timers.h"
#include "clcd.h"
#pragma config WDTE = OFF

void init_config(void)
{
    /*Initialise the Digital Keypad */
    init_digital_keypad();
    
    /*Initialise the clcd*/
    init_clcd();
    
    /*config RC2 as the output pin*/
    FAN_DDR = 0;
    /*Config RC1 as output*/
    BUZZER_DDR = 0;
    BUZZER = OFF;
    
    init_timer2();
    
     PEIE = 1;
     GIE = 1;

}

/*Initially the operation mode is Washing Program*/
unsigned char operation_mode = WASHING_PROGRAM_DISPLAY;
unsigned char reset_mode = WASHING_PROGRAM_DISPLAY_RESET; 
unsigned char program_no = 0 , water_level_index = 0;
unsigned char *washing_program[] = {"Daily","Heavy","Delicates","Whites","Stainwash","EcoCottons","Woolens","Bedsheets","Rinse+Dry","Dry only","Wash only","Aqua store"};
unsigned char *water_level_options[] = {"Auto","Low","Medium","High","Max"};
                                        // 0       1      2       3      4

unsigned char min,sec;
unsigned int rinse_time, wash_time, spin_time;
void main(void) {
    init_config();
    unsigned char key ;
    /*Display the keys status*/
    clcd_print("Press Key5 To",LINE1(1));
    clcd_print("Power ON ",LINE2(4));
    clcd_print("Washing Machine",LINE3(1));
    
    /*Keep waiting till the switch5 is pressed*/
   while(read_digital_keypad(STATE) != SW5)
   {
       for(unsigned int wait = 3000;wait--;);
   }
    
   power_on_screen();
    
    while(1) 
    {
        key = read_digital_keypad(STATE); /*SW5 SW6 .... ALL_RELEASED*/
        //If S4 is pressed for long time if the screen is in washing mode
        for(unsigned int wait = 3000; wait--;);
    if(key == LSW4 && operation_mode == WASHING_PROGRAM_DISPLAY)
    {
        operation_mode = WATER_LEVEL;
        reset_mode = WATER_LEVEL_RESET;
    }
        // If sw4 is pressed for long time, operation mode is water level screen
    else if(key == LSW4 && operation_mode == WATER_LEVEL)
        {
            set_time();
            // Change operation mode
            operation_mode = START_STOP_SCREEN;
            clear_screen();
            clcd_print("Press Switch",LINE1(1));
            clcd_print("SW5: START",LINE2(1));
            clcd_print("SW6: STOP",LINE3(1));
            
        }
        // operation mode is pause sw5 is pressed
        if(key == SW5 && operation_mode == PAUSE)
        {
            TMR2ON = 1;
            FAN = ON;
            operation_mode = START_PROGRAM;
        }
        
       switch (operation_mode)
       {
           case WASHING_PROGRAM_DISPLAY:
               washing_program_display(key);
               break;
           case WATER_LEVEL:
               water_level_display(key);
               break;
           case START_STOP_SCREEN:
               if(key == SW5)
               {
                   operation_mode = START_PROGRAM;
                   reset_mode = START_PROGRAM_RESET;
                   
               }
               else if(key == SW6)
               {
                
                   operation_mode =  WASHING_PROGRAM_DISPLAY;
                    reset_mode = WASHING_PROGRAM_DISPLAY_RESET;
                    continue;
               }
               break;
               
           case START_PROGRAM:
                run_program(key);
               break;
       }
       reset_mode = RESET_NOTHING;

    }

    
}
 void power_on_screen(void)

{
    /*To print Block*/
    for(unsigned char i = 0; i < 16; i++)
    { 
    clcd_putch(BLOCK,LINE1(i));
    }
    clcd_print("Powering On",LINE2(2));
    clcd_print("Washing Machine",LINE3(1));
    for(unsigned char i = 0; i < 16; i++)
    { 
    clcd_putch(BLOCK,LINE4(i));
    }
      __delay_ms(1000);
    
      clear_screen();
}
 
 void washing_program_display(unsigned char key)
{
     if(reset_mode == WASHING_PROGRAM_DISPLAY_RESET)
     {
      clear_screen();
      program_no = 0;
     }
     /*to check if SW4 is pressed increment the index of programs */
     if(key == SW4)
     {
         program_no++;
         clear_screen();
         if( program_no == 12)
         {
             program_no = 0;
         }
     }
     clcd_print("Washing Programs",LINE1(0));
     clcd_print('*',LINE2(0));
     
     /*Program is 12 daily , heavy ....... */
     // program_no = 0 -> daily,heavy,delicates
     //program_no = 1 -> heavy,delicates,whites
     if(program_no <= 9)
     {
     clcd_print(washing_program[program_no],LINE2(2));
     clcd_print(washing_program[program_no + 1],LINE3(2));
     clcd_print(washing_program[program_no + 2],LINE4(2));
     }
     // wash only , aquastore, daily
     else if (program_no == 10)
     {
     clcd_print(washing_program[program_no],LINE2(2));
     clcd_print(washing_program[program_no + 1],LINE3(2));
     clcd_print(washing_program[0],LINE4(2));
     }
     //aquastore , daily , heavy
     else if (program_no == 11)
     {
     clcd_print(washing_program[program_no],LINE2(2));
     clcd_print(washing_program[0],LINE3(2));
     clcd_print(washing_program[1],LINE4(2));
     }
     
    
  
 }
 void water_level_display(unsigned char key)
 {
     if(reset_mode == WATER_LEVEL_RESET)
     {
         water_level_index = 0;
         clear_screen();
     }
     if(key == SW4)
     {
         water_level_index++;
         if(water_level_index++ == 5)
         {
             water_level_index = 0;
         }
         clear_screen();
         //increment the water level options
     }
     //print the options based on sw press
     clcd_print("Water Level",LINE1(0));
     clcd_putch('*',LINE2(0));
     
     // Auto Low Medium -> 0
     // Low Medium High -> 1
     // Medium High Max -> 2
     // High Max Auto -> 3
     // Max Auto Low -> 4
     
     if(water_level_index <= 2)
     {
         clcd_print(water_level_options[water_level_index],LINE2(2));
         clcd_print(water_level_options[water_level_index + 1],LINE3(2));
         clcd_print(water_level_options[water_level_index + 2],LINE4(2));
         
     }
     else if (water_level_index == 3)
     {
         clcd_print(water_level_options[water_level_index],LINE2(2));
         clcd_print(water_level_options[water_level_index + 1],LINE3(2));
         clcd_print(water_level_options[0],LINE4(2));  
         
     }
     else if (water_level_index == 4)
     {
         clcd_print(water_level_options[water_level_index],LINE2(2));
         clcd_print(water_level_options[0],LINE3(2));
         clcd_print(water_level_options[1],LINE4(2));  
         
     }
     
 }
 void set_time(void)
 {
     switch(program_no)
     {
         //delay
         case 0:
             switch(water_level_index)
             {
                 //low
                 case 1:
                     sec = 33;
                     min = 0;
                     break;
                     //Auto medium
                 case 0:
                 case 2:
                     sec = 41;
                     min = 0;
                     break;
                     // High max
                 case 3:
                 case 4:
                     sec = 45;
                     min = 0;
                     break;
                     
             }
             break;
             // Heavy
         case 1:
             switch(water_level_index)
             {         
             case 1:
                     sec = 43;
                     min = 0;
                     break;
                     //Auto medium
                 case 0:
                 case 2:
                     sec = 50;
                     min = 0;
                     break;
                     // High max
                 case 3:
                 case 4:
                     sec = 57;
                     min = 0;
                    break;
             }
             break;
         case 2:
         {
             switch(water_level_index)
             {
                 //Low medium auto
             case 1:
                 case 0:
                 case 2:
                     sec = 26;
                     min = 0;
                     break;
                     // High max
                 case 3:
                 case 4:
                     sec = 51;
                     min = 0;
                    break;
             }
             break;
         }
         // Whites
         case 3:
         {
           sec = 16;
           min = 1;
           break;
         }
         case 4: //Stain Wash
         {
           sec = 36;
           min = 1;
           break;
         }
         case 5:// Eco Cottons
         {
           sec = 36;
           min = 0;
           break;
         }
         case 6: // Woollens
         {
           sec = 29;
           min = 0;
           break;
         }
         case 7: // Bedsheets
         {
           switch(water_level_index)
             {         
             case 1:
                     sec = 46;
                     min = 0;
                     break;
                     //Auto medium
                 case 0:
                 case 2:
                     sec = 53;
                     min = 0;
                     break;
                     // High max
                 case 3:
                 case 4:
                     sec = 00;
                     min = 1;
                    break;
             }
           break;
         }
         case 8: // Rinse and Dry
         {
             switch(water_level_index)
             {         
             case 1:
                     sec = 18;
                     min = 0;
                     break;
                     //Auto medium
                 case 0:
                 case 2:
                     // High max
                 case 3:
                 case 4:
                     sec = 20;
                     min = 0;
                    break;
             }
             break;
         }
         case 9: // Dry
         {
           sec = 6;
           min = 0;
           break;
             
         }
         case 10:
         case 11: // Wash and Aqua Store
         {
             switch(water_level_index)
             {         
             case 1:
                     sec = 16;
                     min = 0;
                     break;
                     //Auto medium
                 case 0:
                 case 2:
                     sec = 21;
                     min = 0;
                     break;
                     // High max
                 case 3:
                 case 4:
                     sec = 26;
                     min = 0;
                    break;
             }
             break;
         }
     }
     
 }
     void run_program(unsigned char key)
     {
         door_status_check();
         static int total_time, time;
         if(reset_mode == START_PROGRAM_RESET)
         {
             clear_screen();
             clcd_print("Prog:",LINE1(0));
             clcd_print(washing_program[program_no],LINE1(6));
             clcd_print("Time:",LINE2(0));
             
             //Display time taken by the functions in seconds and minutes
             clcd_print((min/10)+'0',LINE2(6));
             clcd_putch((min%10)+'0',LINE2(7));
             clcd_putch(':',LINE2(8));
             clcd_putch((sec/10)+'0',LINE2(9));
             clcd_putch((sec%10)+'0',LINE2(10));
             clcd_print("(MM:SS)",LINE3(5));
             
             __delay_ms(2000); // 2 sec
             clear_screen();
             clcd_print("Function - ",LINE1(0));
             clcd_print("TIME: ",LINE2(0));
             clcd_print("5-START  6-PAUSE ",LINE4(0));
             
             time = total_time = (min*60) + sec;//100
             wash_time = (int)total_time*(0.46);//46
             rinse_time = (int)total_time*(0.12);//12
             spin_time = total_time - wash_time - rinse_time;
             
          /*to turn On the fan */
             FAN = ON;
          /*Turn on the timer*/
             TMR2ON = 1;
         }
         if(key == SW6)
         {
             FAN = OFF;
             TMR2ON = 0;
             operation_mode = PAUSE;
         }
         
         total_time = (min*60) + sec ;
         if(program_no <= 7)
         {
             if(total_time >= (time - wash_time))// 100-46 -> 54
             {
                 clcd_print ("Wash", LINE1(11));
             }
             else if(total_time >= (time - wash_time - spin_time)) //12 100-46-42
             {
                 clcd_print("Spin",LINE1(11));
             }
             else
             {
                 clcd_print("Spin",LINE1(11));
             }
         }
         else if(program_no == 0)// rinse+dry
         {
             if(total_time >= time - (0.40 *time))
             {
                 clcd_print("Rinse",LINE1(11));
                 
             }
             else
             {
                 clcd_print("Spin",LINE1(11));
                 
             }
         }
         else if(program_no == 9)
         {
             clcd_print("Spin",LINE1(11));
         }
         else
         {
             clcd_print("Wash",LINE1(11));
             
         }
         clcd_print((min/10)+'0',LINE2(6));
         clcd_putch((min%10)+'0',LINE2(7));
         clcd_putch(':',LINE2(8));
         clcd_putch((sec/10)+'0',LINE2(9));
         clcd_putch((sec%10)+'0',LINE2(10));
         
         if ((sec==0) && (min==0))
         {
             /*To turn ON the FAN*/
             FAN = OFF;
             /*Turn On the Timer*/
             TMR2ON = OFF;
             BUZZER = ON;
             clear_screen();
             clcd_print("Program Completed",LINE1(0));
             clcd_print("Remove Clothes",LINE2(0));
             __delay_ms(2000);
             BUZZER = OFF;
             operation_mode = WASHING_PROGRAM_DISPLAY;
             reset_mode = WASHING_PROGRAM_DISPLAY_RESET;
             clear_screen();
             
         }
     }
     void door_status_check(void)
     {
         if(RB0 == 0) //if door is open
         {
             // stop the machine and turn on the buzzer
             FAN = OFF;
             TMR2ON = OFF;
             BUZZER = ON;
             clear_screen();
             clcd_print("Door : open",LINE1(0));
             clcd_print("Please Close",LINE3(0));
             while(RB0 == 0)
             {
                  ;
             }
              clear_screen();
             clcd_print("Function - ",LINE1(0));
             clcd_print("TIME: ",LINE2(0));
             clcd_print("5-START  6-PAUSE ",LINE4(0));
             /*to turn On the fan */
             FAN = ON;
          /*Turn on the timer*/
             TMR2ON = 1;
             BUZZER = OFF;
         }
     }
     
     
     
 