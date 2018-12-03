#include <fontALL.h>
#include <video_gen.h>
#include <TVout.h>

TVout TV;

// adc0 = controller one paddle
// adc1 = controller one spin
// pin 2 = controller one reset
// adc3 = controller two paddle
// adc4 = controller two spin
// pin 3 = controller 2 reset
// pin 9 = sync
// pin 7 = video
// pin 11 = audio

#define PADDLE_HEIGHT 8 	        //Defines paddles physical height in blocks. Note that blocks do not correlate to individual pixel sizes. 
#define PADDLE_WIDTH 1		//Defines paddle physical width in  blocks. Note that blocks do not correlate to individual pixel sizes.	

#define LEFT_PADDLE_X_AXIS_POSITION 2                    // This ensures the left paddle is placed slightly to the right of the game boundry. 
#define RIGHT_PADDLE_X_AXIS_POSITION (TV.hres()-2)	// This ensures the right paddle is placed slightly to the left of the game boundry.

//Sets up positioning of game score text.
#define LEFT_SCORE_X_AXIS_POSITION (TV.hres()/2 -15)
#define LEFT_SCORE_Y_AXIS_POSITION 4
#define RIGHT_SCORE_X_AXIS_POSITION (TV.hres()/2 +13)
#define RIGHT_SCORE_Y_AXIS_POSITION 4

#define MAX_BALL_Y_VELOCITY 2 //Defines maximum y velocity for ball. Here so ball doesn't move too fast.

// Defines arduino adc pins for controllers.
#define CONTROLLER_ONE_PADDLE_PIN 2
#define CONTROLLER_ONE_SPIN_PIN 3
#define CONTROLLER_ONE_RESET_PIN 2
#define CONTROLLER_TWO_PADDLE_PIN 0
#define CONTROLLER_TWO_SPIN_PIN 1
#define CONTROLLER_TWO_RESET_PIN 3

// Defines winning game score, will go to menu mode after score is reached.
#define WINNING_SCORE 11


uint16_t controller_one_paddle_position = 0;
int16_t controller_one_spin = 0;
uint8_t controller_one_reset = 0;
uint16_t controller_two_paddle_position = 0;
int16_t controller_two_spin = 0;
uint8_t controller_two_reset = 0;
int16_t previous_controller_one_spin = 0; //Must get initial adc values before game starts.
int16_t previous_controller_two_spin = 0; //Must get initial adc values before game starts.
uint16_t controller_one_spin_value = 0;
uint16_t controller_two_spin_value = 0;

uint16_t left_paddle_y_axis_position = 0;
uint16_t left_paddle_x_axis_position = LEFT_PADDLE_X_AXIS_POSITION;
uint16_t right_paddle_y_axis_position = 0;
uint16_t right_paddle_x_axis_position = RIGHT_PADDLE_X_AXIS_POSITION;

uint16_t ball_x_position = 0;
int16_t ball_y_position = 0;
int8_t ball_x_velocity = 1; 
int8_t ball_y_velocity = 1; 

uint8_t player_one_score = 0;
uint8_t player_two_score = 0;

uint8_t impact_noise = false;        //Flag to tell program when to play impact sound. Needed so sound only plays once screen has been updated.
uint8_t point_scored_noise = false;  //Flag to tell program when to play point scored sound. Needed so sound only plays once screen has been updated.

uint8_t menu_mode = true;            //Flag to tell program when to be in menu mode, and when to be be simply playing the game.

uint8_t wall_hit = false;            //Flag to tell program if the ball has hit the top or bottom wall recently. This is here to ensure a player can only
                                     //spin the wall before it has hit a wall.
                                     
uint8_t game_reset_wait = false;     //Flag to tell game that the reset button has not yet been pressed after a point has been scored.


// This function collects input data from the controllers, and processes the spin data.
void get_controller_inputs() //processInputs
{
	previous_controller_one_spin = controller_one_spin;
	previous_controller_two_spin = controller_two_spin;
	controller_one_paddle_position = analogRead(CONTROLLER_ONE_PADDLE_PIN);
	controller_one_spin = analogRead(CONTROLLER_ONE_SPIN_PIN);
	controller_one_reset = digitalRead(CONTROLLER_ONE_RESET_PIN);
	controller_two_paddle_position = analogRead(CONTROLLER_TWO_PADDLE_PIN);
	controller_two_spin = analogRead(CONTROLLER_TWO_SPIN_PIN);
	controller_two_reset = digitalRead(CONTROLLER_TWO_RESET_PIN);

        //This piece of code deals with reseting the wall after a point has been scored.
        if(controller_one_reset || controller_two_reset)
        {
          game_reset_wait = false;
        }
         
          // This piece of code processes the spin attributes.
         if(!wall_hit && !menu_mode)
         {
          if(ball_x_velocity >0 )        //Tests if ball just hit left paddle.
          {
              if( (controller_one_spin-previous_controller_one_spin) > 20)
              {
                ball_y_velocity--;
                if(ball_y_velocity <-2)
                {
                  ball_y_velocity = -2;
                }
              }
              else if( (controller_one_spin-previous_controller_one_spin) < -20)
              {
                ball_y_velocity++;
                if(ball_y_velocity >2)
                {
                  ball_y_velocity = 2;
                }
              }
            }
          
            if(ball_x_velocity <0)        //Tests if ball just hit left paddle.
            {
              if( (controller_two_spin-previous_controller_two_spin) > 20)
              {
                ball_y_velocity--;
                if(ball_y_velocity <-2)
                {
                  ball_y_velocity = -2;
                }
              }
              else if( (controller_two_spin-previous_controller_two_spin) < -20)
              {
                ball_y_velocity++;
                if(ball_y_velocity >2)
                {
                  ball_y_velocity = 2;
                }
              }
            }
         }        

}
	
//This function prints the game, score, paddle and ball lines.
void update_game_screen() //drawGameScreen
{  
        TV.clear_screen();
        //drawing middle line
         draw_game_boundry();
         
         //drawing ball
         if(!game_reset_wait)
         {
            TV.draw_rect(ball_x_position,ball_y_position,2,2,1,1);

          // TV.set_pixel(ball_x_position, ball_y_position, 1);
         }
         
        if(!menu_mode)
        {
	  left_paddle_y_axis_position = ( (controller_one_paddle_position/8)*(TV.vres()-PADDLE_HEIGHT) ) / 128;
	  left_paddle_x_axis_position = LEFT_PADDLE_X_AXIS_POSITION;
            //Draw left Paddle
           TV.draw_rect(left_paddle_x_axis_position,left_paddle_y_axis_position,1,PADDLE_HEIGHT,1,1);

      	  right_paddle_y_axis_position = ( (controller_two_paddle_position/8)*(TV.vres()-PADDLE_HEIGHT) ) / 128;
      	  right_paddle_x_axis_position = RIGHT_PADDLE_X_AXIS_POSITION;
      	   
            //Draw right paddle
           TV.draw_rect(right_paddle_x_axis_position,right_paddle_y_axis_position,1,PADDLE_HEIGHT,1,1);

        }
       //drawing score
	TV.print_char(LEFT_SCORE_X_AXIS_POSITION,LEFT_SCORE_Y_AXIS_POSITION,'0'+player_one_score);
	TV.print_char(RIGHT_SCORE_X_AXIS_POSITION,RIGHT_SCORE_Y_AXIS_POSITION,'0'+player_two_score);
  
}

// This function draws the middle line in the game.
void draw_game_boundry()
{
        TV.draw_rect((TV.hres()/2),0,1,TV.vres(),1,1);
}

// This function enables menu mode. No points can be scored, paddles do not display, and the ball bounces around all corners of the screen.
// This function is exited only once a controller reset button has been pressed.
void menu()
{
        ball_x_position = TV.hres()/2;
        ball_x_velocity = 1;
        ball_y_velocity = 1;
        while(!controller_one_reset && !controller_two_reset)
        {
          ball_x_position += ball_x_velocity;
	  ball_y_position += ball_y_velocity;
	
	  // Test if ball has colided with top or bottom of screen.
	  if( ((ball_y_position <= 1) && (ball_y_velocity < 0)) || (ball_y_position >= TV.vres()-2 && ball_y_velocity > 0) )
	  {
	    ball_y_velocity = -ball_y_velocity; // inverts balls vertical direction.
            impact_noise = true;	//makes wall impact noise.

	  }

          if( (ball_x_position <= 1) || (ball_x_position >= TV.hres()-2) ) 
	  {
            ball_x_velocity = -ball_x_velocity; //Inverts velocity to bring ball back in to play.
            impact_noise = true;	//makes wall impact noise.

	  }
          get_controller_inputs();
          update_game_screen();
            if(impact_noise)
          {
            TV.tone(2000,30); // Makes impact noise.
            impact_noise = false;
          }
          delay(5);
        //  TV.delay_frame(1);
        }
        ball_x_position = TV.hres()/2;
        player_one_score = 0;
        player_two_score = 0;
        menu_mode = false;
}
  
//This function hads to a players score if they have scored.
void point_scored(uint8_t player)
{
	if(player == 1)
	{
		player_one_score++;
	}
	if(player == 2)
	{
		player_two_score++;
	}
}
void setup()
{
	TV.begin(_NTSC);                  // Should maybe be PAL...
        TV.select_font(font4x6);
	
	ball_x_position = TV.hres() / 2; //Starts ball in centre.
	ball_y_position = TV.vres() / 2; //Starts ball in centre.
	
	pinMode(CONTROLLER_ONE_RESET_PIN, INPUT);	// Sets up reset pins as inputs
	pinMode(CONTROLLER_TWO_RESET_PIN, INPUT);

	//This must be read before game starts to ensure spin calculations are computed correctly.

	controller_one_spin = analogRead(CONTROLLER_ONE_SPIN_PIN);
	controller_two_spin = analogRead(CONTROLLER_TWO_SPIN_PIN);

        draw_game_boundry();
}


void loop()
{
  if(menu_mode)
  {
    menu();
  }
  else
  {
    get_controller_inputs();
    if(!game_reset_wait)
    {
      ball_x_position += ball_x_velocity;
      ball_y_position += ball_y_velocity;
    }
   
    // Test if ball has colided with top or bottom of screen.
    if( ((ball_y_position <= 1) && (ball_y_velocity < 0)) || (ball_y_position >= TV.vres()-2 && ball_y_velocity > 0) )
    {
      ball_y_velocity = -ball_y_velocity; // inverts balls vertical direction.
      impact_noise = true;	//makes wall impact noise.
      wall_hit = true;
    }
    // Test if ball has hit player one paddle.
    if( (ball_x_velocity < 1) && (ball_x_position <= LEFT_PADDLE_X_AXIS_POSITION+PADDLE_WIDTH) && (ball_y_position >= left_paddle_y_axis_position-1) && (ball_y_position <= left_paddle_y_axis_position + PADDLE_HEIGHT) )
    {
      ball_x_velocity = -ball_x_velocity;	// Inverts ball x axis velocity.
      
        if((ball_y_position - left_paddle_y_axis_position) > 4) 
        ball_y_velocity++;
        else if( (ball_y_position - left_paddle_y_axis_position) < 4)
        ball_y_velocity++; 
      
      impact_noise = true;	
      wall_hit = false;
    }
    // Test if ball has hit player two paddle.
    if( (ball_x_velocity > 0) && (ball_x_position >= RIGHT_PADDLE_X_AXIS_POSITION-PADDLE_WIDTH) && (ball_y_position >= right_paddle_y_axis_position-1) && (ball_y_position <= right_paddle_y_axis_position + PADDLE_HEIGHT) )
    {
      ball_x_velocity = -ball_x_velocity;	// Inverts ball x axis velocity.
      
        if((ball_y_position - left_paddle_y_axis_position) > 4) 
        {
           ball_y_velocity++;
            if(ball_x_velocity > 0) 
          ball_x_velocity++;
          else
          ball_x_velocity--;
        }
        else if( (ball_y_position - left_paddle_y_axis_position) < 4)
        {
          ball_y_velocity--;
         if(ball_x_velocity > 0) 
          ball_x_velocity++;
          else
          ball_x_velocity--;
        }
        impact_noise = true; // Makes impact noise.
      wall_hit = false;
    }
    //Limit the vertical speed which can acrew over time.
    if(ball_y_velocity > MAX_BALL_Y_VELOCITY) 
    {
      ball_y_velocity = MAX_BALL_Y_VELOCITY;
    }
    if(ball_y_velocity < -MAX_BALL_Y_VELOCITY) 
    {
      ball_y_velocity = -MAX_BALL_Y_VELOCITY;
    }
    //Test if a point has been scored.
    if(ball_x_position <= 1)
    {
      point_scored(2);
      point_scored_noise = true; //sound for point scored.
      ball_x_position = TV.hres()/2;;
      ball_x_velocity = 1; //Inverts velocity to bring ball back in to play.
      ball_y_velocity = 1;
      wall_hit = false;
      if(player_two_score >= 11)
      {
        menu_mode = true;
      }
      game_reset_wait = true;
    }
    if(ball_x_position >= TV.hres()-1)
    {
      point_scored(1);
      point_scored_noise = true; //sound for point scored.
      ball_x_position = TV.hres()/2;
      ball_x_velocity = -1; //Inverts velocity to bring ball back in to play.
      ball_y_velocity = 1;
      wall_hit = false;
      if(player_one_score >= 11)
      {
        menu_mode = true;
      }
      game_reset_wait = true;
    }

    update_game_screen();
    if(impact_noise)
    {
      TV.tone(2000,30); // Makes impact noise.
      impact_noise = false;
    }
    else if(point_scored_noise)
    {
      TV.tone(500,300);
      point_scored_noise = false;
    }

     TV.delay_frame(1);
             delay(5);  // Increase or decrease this to change game speed.



  }
}
		
		


		


