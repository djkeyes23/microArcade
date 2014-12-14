//MircroArcade EECS 183 2014 Showcase
//Breaker Breaker

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "stdlib.h"
#include "math.h"
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

class Board		//Contains necessary values to keep track of the game
{
public:
  Board(){
    paddlePos = 0;
    lives = 3;
    level = 0;
    paddleHeight = 7;
    paused = true;
    ballLeft = true;
  } 
  int strength[8][16];	//The strengths of blocks remaining on the board

  int getPaddlePos(){
    return paddlePos;
  }
  void setPaddlePos(int input){
    paddlePos = input;
  }

  int getLives(){
    return lives;
  }
  void setLives(int input){
    lives = input;
  }

  int getLevel(){
    return level;
  }
  void setLevel(int input){
    level = input;
  }

  int getBallRow(){
    return ballRow;
  }
  void setBallRow(int input){
    ballRow = input;
  }

  int getBallCol(){
    return ballCol;
  }
  void setBallCol(int input){
    ballCol = input;
  }

  boolean getBallLeft(){
    return ballLeft;
  }
  void setBallLeft(boolean input){
    ballLeft = input;
  }

  boolean getBallRight(){
    return ballRight;
  }
  void setBallRight(boolean input){
    ballRight = input;
  }

  boolean getBallDown(){
    return ballDown;
  }
  void setBallDown(boolean input){
    ballDown = input;
  }

  int getPaddleHeight() {
    return paddleHeight;
  }

  void resetPause() {
    paused = false;
  }
  void setPause() {
    paused = true;
  }
  boolean getPaused() {
    return paused;
  }
  
  //MODIFIES: nothing 
  //EFFECTS: returns BlockHits
  int getBlockHits();

  //MODIFIES: BlockHits 
  //EFFECTS: sets BlockHits to input
   int setBlockHits(int input);


  //MODIFIES: strength array, ballRow, ballCol, BallRight, ballDown
  //EFFECTS: first sets up the ball so that it is on top of the paddle
  // on the left pixel with initial direction to the left and up. Next
  //sets up the board according to which level the game is on.
  void initBoard();

  //EFFECTS:Draws the pixels with strength>0 to the board
  void displayBlocks(  );

  //MODIFIES: ballRow and ballCol
  //EFFECTS: updates the balls position based on what direction the ball
  //is traveling. Be careful to not let the ball go off the board!
  void updateBall();


  //MODIFIES: nothing
  //EFFECTS: returns true if the ball goes off the board when the player
  //fails to hit it with the paddle
  boolean lostBall(  );

  //MODIFIES:ballDown, ballRight, ballLeft
  //EFFECTS: changes the ball's direction if the ball hits the top or sides of the board
  boolean hitWall();

  //MODIFIES:ballDown, ballRight, ballLeft
  //EFFECTS:changes the direction of the ball depending on which direction the ball was traveling
  //when it hits the paddle
  boolean hitPaddle( );

  //MODIFIES: nothing
  //EFFECTS: returns true if all blocks are destroyed, false otherwise 
  boolean levelComplete();

  //MODIFIES:ballDown, ballRight, ballLeft, strength array
  //EFFECTS:changes the direction of the ball depending on which direction the ball was traveling
  //when it hits a block. Decrements the strength of the block.
  boolean hitBlock();

private:
  int blockHits;
  int paddlePos;		//Leftmost position of the paddle
  int paddleHeight;	// height of the upper paddle
  int lives;			//Number of lives left
  int level;			//Current level
  int ballRow;			//Row of the ball
  int ballCol;			//Column of the ball
  boolean ballLeft;		//If ballleft == TRUE, ball is moving left
  boolean ballRight;		//If ballRight == TRUE, ball is moving right.  
  boolean ballDown;		//If ballDown == TRUE, ball is moving down.  Otherwise, ball is moving up
  boolean paused;			// Pause after ball loss

};


//EFFECTS:displays the number of lives remaining/level number on the board. If it doesn't all fit
//on the board at once, the text scrolls.
//use text size 1 and delay of 50
void printMessage(int number);

//EFFECTS:displays string text needed(suchs as "LIVES" or "LEVEL") on the board. If it
//doesn't all fit on the board at once, the text scrolls.
//use text size 1 and delay of 50
void printMessage(String message);

// Matrix class instance to display all game objects to 8x16 LED Adafruit matrix
Adafruit_8x16matrix matrix = Adafruit_8x16matrix();

// struct to hold all relevant IO pin data
struct Pins{

  int potPin; 	        //input pin potentiometer
  int val;
  int smoothval;		//value of the potentiometer
  int button;		//input pin for the button
  int buttonState; // current state of the button
  
};

Board board;				// declare playing field
Pins pins;
					// pins to be used for game input
int   sensorPin = 2;                    // sensor input pin
int   rawVal = 0;                       // sensor raw value
float smoothedVal = 0.0;                // sensor smoothed value
float smoothStrength = 2;               // amount of smoothing (default 10)



void setup()
{
	String introMessage = "                Breaker Breaker Master";
	lcd.begin(16, 2); // set up the LCD's number of columns and rows: 
	lcd.clear();
	lcd.print(introMessage); // Print intro message to the LCD.
	delay(100); //set to 100

	for (int positionCounter = 0; positionCounter < 38; positionCounter++)
	{
		lcd.scrollDisplayLeft(); // scroll one position left:
		delay(300); // wait a bit, for LCD refresh set to 300
	}

  lcd.clear();
	
  pins.potPin=2;
  pins.val=0;
  pins.button=7; 
  pinMode(pins.button, INPUT);				//Sets the buttons pin to be an input pin
  matrix.begin(0x70);  				// pass in the address
  clearBoard();			//Set entire board to 'off'
  initStrength(board.strength);                    //set all strengths to 0	

}

void loop()
{
        rawVal = analogRead(sensorPin);
        smoothedVal = smooth(rawVal, smoothedVal);
    
        int gameTime = ((millis() / 1000) - 11);
	int score = ((board.getBlockHits()*100) - gameTime);
	int lives = board.getLives();
	int level = board.getLevel();

	lcd.setCursor(0, 0);

	lcd.print("Score:");
	lcd.print(score);

	lcd.setCursor(13, 0);
	lcd.print("L:");
	lcd.print(lives);

	lcd.setCursor(0, 1);
	lcd.print("T:");
	lcd.print(gameTime);

    lcd.setCursor(11, 1);
    lcd.print("Lvl:");
    lcd.print(level);

  // set all pixels on the game board to LED_OFF
  clearBoard();

  // read the current value of the analog input for the paddle
  pins.val = smoothedVal;  // get value from sensor

  // read the current value of the button
  pins.buttonState = digitalRead(pins.button);

  // If the button is HIGH, reset the pause state
  if(pins.buttonState) {
    board.resetPause();
  }

  // update the position of the paddle based on the input range
  board.setPaddlePos(calculatePaddlePosition(pins.val));

  // draw the lower paddle on the board
  matrix.drawPixel( board.getPaddlePos(), 15, LED_ON);
  matrix.drawPixel( board.getPaddlePos() + 1, 15, LED_ON);  //Turns on the LEDs indicating the paddle

  // draw the upper paddle on the board
  matrix.drawPixel( board.getPaddlePos(), board.getPaddleHeight(), LED_ON);
  matrix.drawPixel( board.getPaddlePos() + 1, board.getPaddleHeight(), LED_ON);  //Turns on the LEDs indicating the paddle

  // check for a board with no blocks (strength for all pixels == 0)
  if(board.levelComplete()){     //checks if the level is done
    board.setLevel(board.getLevel()+1);			// increment the board level
    board.initBoard();							// initialize the state of the board for the new level

    // pause the game to start a new level
    board.setPause();
  }

  // detect collisions with objects
  board.hitWall();
  board.hitBlock();
  board.hitPaddle();

  // detect collisions with objects - AGAIN
  // this is done to make sure the ball detects another collision after
  // striking a different object and changing direction without changing position yet
  board.hitWall();
  board.hitBlock();
  board.hitPaddle();

  // detect if the ball has left the palying field (has hit the bottom of the playing field
  if( board.lostBall() == true ) {				// checks if ball is lost
    board.setLives(board.getLives()-1);			// decrement the number of lives 

    if(board.getLives() <=0 ) {					// check for game over
      gameOver();
    }
    else {
      // display a flashing message to indicate the ball was lost
      printMessage(board.getLives());
      printMessage(" lives");		
      board.setBallRow(14);						//if lives are left, resets the ball to be above the lower paddle    
      board.setBallCol(board.getPaddlePos());	// put the ball over the left side of the paddle
      board.setBallRight(false);				// set the direction the ball is to go
      board.setBallDown(false);
      board.setBallLeft(true);

      // pause the game to allow the player to get ready
      board.setPause();

    }											// end of else... getLives() > 0

  }												// end of if board.lostBall == true

    // if the game is paused, don't update state
  if(!board.getPaused()) {
    // update the position of the ball
    board.updateBall();
  }
  else 
  {
    board.setBallCol(board.getPaddlePos());
  }

  // draw the ball on the screen
  matrix.drawPixel(board.getBallCol(), board.getBallRow(), LED_ON);

  // display all blocks to the display
  board.displayBlocks();
  matrix.writeDisplay(); //display all changes made in one iteration of loop
  delay(150); //to slow it down and make it easier to debug. also makes the paddle lag       

}												// end of loop()


void clearBoard(  )
{
  for( int j = 0; j < 8; j++) {
    for( int i = 0; i < 16; i++ ) {
      matrix.drawPixel(j, i, LED_OFF);
    }
  }
}												// end of clearBoard()

void initStrength(int strength[][16])
{
  for (int r = 0; r < 16; r++) {		//Set strengths of all blocks to 0
    for(int c = 0; c < 8; c++) {
      board.strength[c][r]=0;
    }
  }
}												// end of initStrength


boolean Board::hitBlock()
{
  if(ballRight){
    if(ballDown){
      if(ballRow+1<16 && ballCol+1<8 && strength[ballCol+1][ballRow+1]>0){
        strength[ballCol+1][ballRow+1]--;
        blockHits++;
        if( ((ballCol+1) % 2) == 0) {
          strength[ballCol+2][ballRow+1]--;
        }
        else {
          strength[ballCol][ballRow+1]--;
        }
        ballDown=!ballDown;
        ballRight= false;
        ballLeft = true;
        //  Serial.print( strength[ballRow+1][ballCol+1]);
        Serial.println(" block strength");
      }
    }
    else{
      if(ballRow-1 >=0 && ballCol+1<8 && strength[ballCol+1][ballRow-1]>0){
        strength[ballCol+1][ballRow-1]--;
        blockHits++;
        if( ((ballCol+1) % 2) == 0) {
          strength[ballCol+2][ballRow-1]--;
        }
        else {
          strength[ballCol][ballRow-1]--;
        }

        ballDown=!ballDown;
        ballRight= false;
        ballLeft = true;
        // Serial.print( strength[ballRow-1][ballCol+1]);
        Serial.println(" block strength1");
      }
    }
  }
  else if(ballLeft) {
    if(ballDown){
      if((ballRow+1<16) && (ballCol-1>=0) && (strength[ballCol-1][ballRow+1]>0)){
        //Serial.print( strength[ballCol-1][ballRow+1]);
        Serial.println(" block strength2");
        strength[ballCol-1][ballRow+1]--;
        blockHits++;
        if( ((ballCol-1) % 2) == 0) {
          strength[ballCol][ballRow+1]--;
        }
        else {
          strength[ballCol-2][ballRow+1]--;
        }
        ballDown=!ballDown;
        ballRight= true;
        ballLeft = false;
        //Serial.print( strength[ballRow+1][ballCol-1]);
        Serial.println(" block strength2");
      }
    }
    else{
      if(ballRow-1>=0 && ballCol-1>=0 && strength[ballCol-1][ballRow-1]>0){
        strength[ballCol-1][ballRow-1]--;
        blockHits++;
        if( ((ballCol-1) % 2) == 0) {
          strength[ballCol][ballRow-1]--;
        }
        else {
          strength[ballCol-2][ballRow-1]--;
        }
        ballDown=!ballDown;
        ballRight= true;
        ballLeft = false;
        // Serial.print( strength[ballRow-1][ballCol-1]);
        Serial.println(" block strength3");
      }
    }
  }
  else
  {
    if(ballDown){
      if( strength[ballCol][ballRow+1] > 0 ) {
        //Serial.print( strength[ballCol-1][ballRow+1]);
        Serial.println(" block strength2");
        strength[ballCol][ballRow+1]--;
        blockHits++;
        if( ((ballCol) % 2) == 0) {
          strength[ballCol+1][ballRow+1]--;
        }
        else {
          strength[ballCol-1][ballRow+1]--;
        }

        ballDown=!ballDown;
        //Serial.print( strength[ballRow+1][ballCol-1]);
        Serial.println(" block strength2");
      }
    }
    else{
      if( strength[ballCol][ballRow-1] > 0 ) {
        strength[ballCol][ballRow-1]--;
        blockHits++;
        if( ((ballCol) % 2) == 0) {
          strength[ballCol+1][ballRow-1]--;
        }
        else {
          strength[ballCol-1][ballRow-1]--;
        }
        ballDown=!ballDown;
        // Serial.print( strength[ballRow-1][ballCol-1]);
        Serial.println(" block strength3");
      }														// end of if strength > 0								

    }														// end of else... !ballDown		

  }															// end of else... ! (ballRight || ballLeft) 

}															// end of hitBlock()

boolean Board::hitPaddle( )
{
  if( (ballRow == 14 || ballRow == board.getPaddleHeight() -1) && ballDown == true )
  {
    if(ballRight) {
      if( (ballCol == paddlePos - 1) )
      {
        ballDown = !ballDown;
        ballRight = false;
        ballLeft = true;
        Serial.println("hit paddle left");
      }
      else if( ballCol == paddlePos )
      {
        ballDown = !ballDown;
        ballRight = false;
        ballLeft = false;
        Serial.println("hit paddle right");
      }
      else if( ballCol == paddlePos +1)
      {
        ballDown = !ballDown;
        Serial.println("hit paddle right");
      }
    }
    else if(ballLeft)
    {
      if( (ballCol == paddlePos +2) )
      {
        ballDown = !ballDown;
        ballRight = true;
        ballLeft = false;
        Serial.println("hit paddle left");
      }
      else if( ballCol == paddlePos + 1 )
      {
        ballDown = !ballDown;
        ballRight = false;
        ballLeft = false;
        Serial.println("hit paddle right");
      }
      else if( ballCol == paddlePos )
      {
        ballDown = !ballDown;
        Serial.println("hit paddle right");
      }
    }
    else
    {
      if(  ballCol == paddlePos ) {
        ballDown = !ballDown;
        ballRight = false;
        ballLeft = true;
      }
      else if(  ballCol == paddlePos + 1 ) {
        ballDown = !ballDown;
        ballRight = true;
        ballLeft = false;
      }

    }
  }
  else if( (ballRow == board.getPaddleHeight() + 1) && ballDown == false )
  {
    if(ballRight) {
      if( (ballCol == paddlePos - 1) )
      {
        ballDown = !ballDown;
        ballRight = false;
        ballLeft = true;
        Serial.println("hit paddle left");
      }
      else if( ballCol == paddlePos )
      {
        ballDown = !ballDown;
        ballRight = false;
        ballLeft = false;
        Serial.println("hit paddle right");
      }
      else if( ballCol == paddlePos +1)
      {
        ballDown = !ballDown;
        Serial.println("hit paddle right");
      }
    }
    else if(ballLeft)
    {
      if( (ballCol == paddlePos +2) )
      {
        ballDown = !ballDown;
        ballRight = true;
        ballLeft = false;
        Serial.println("hit paddle left");
      }
      else if( ballCol == paddlePos + 1 )
      {
        ballDown = !ballDown;
        ballRight = false;
        ballLeft = false;
        Serial.println("hit paddle right");
      }
      else if( ballCol == paddlePos )
      {
        ballDown = !ballDown;
        Serial.println("hit paddle right");
      }
    }
    else
    {
      if(  ballCol == paddlePos ) {
        ballDown = !ballDown;
        ballRight = false;
        ballLeft = true;
      }
      else if(  ballCol == paddlePos + 1 ) {
        ballDown = !ballDown;
        ballRight = true;
        ballLeft = false;
      }

    }
  }
}													// end of hitPaddle()


//EFFECTS: changes the ball's direction if the ball hits the top or sides
boolean Board::hitWall()
{
  if( ballRow == 0 && !ballDown ) 
  {
    ballDown = !ballDown;
    Serial.println("hit wall");
  }
  if( ballCol == 0 && ballLeft )
  {
    ballRight = true;
    ballLeft = false;
    Serial.println("hit wall");
  }
  if( ballCol == 7 && ballRight )
  {
    ballRight = false;
    ballLeft = true;
    Serial.println("hit wall");
  }
}													// end of hitWall()


//EFFECTS: returns true if the ball goes 
//         off the board because the player 
//         fails to hit it with the paddle
boolean Board::lostBall(  ) 
{

  if( ballRow >= 15 || ballCol < 0 || ballCol > 7) {
    return true; 
  }
  return false;
}


//REQUIRES:  val is the value of the potentiometer
//EFFECTS: returns the column position of the left pixel of the paddle
//MODIFIES: none
int calculatePaddlePosition( int val )
{
  //Since the paddle is two pixels wide, there are 7 possibilities for the leftmost pixel
  if (val >= 0 && val < 359)
  {
    return 0;
  }
  else if (val >= 360 && val < 468)
  {
    return 1;
  }
  else if (val >= 469 && val < 577)
  {
    return 2;
  }
  else if (val >= 578 && val < 687)
  {
    return 3;
  }
  else if (val >= 688 && val < 796)
  {
    return 4;
  }
  else if (val >= 797 && val < 905)
  {
    return 5;
  }
  else
  {
    return 6;
  }
}


//MODIFIES:	modifies the array of ints holding the strength values left in the blocks to be displayed
//EFEFCTS:	set the strength array to hold the new values for the blocks for a new level based on the next level
void Board::initBoard()
{          
  ballRow = 14;
  ballCol=paddlePos;
  ballRight = false;
  ballDown = false;
  ballLeft = true;

  if(level==1) {
    for(int c=0; c<8; c++) {
      strength[c][0]=1;
    }
  }
  else if(level==2) {
    for(int r=0; r<2; r++) {
      for(int c=0; c<8; c++) {
        strength[c][r]=1;
      }
    }
  }
  else if(level==3) {
    for(int c=0; c<8; c++) {
      strength[c][0] = 2;
      strength[c][1] = 1;
    }	
  }
  else if(level==4) {
    for(int c=0; c<8;c++) {
      strength[c][0] = 3;
      strength[c][1] = 2;
      strength[c][2] = 1;
    }
  }
  //Rows of descending strength 3 - 2 - 1, with gaps in between rows
  else if(level==5) {
    for (int c=0; c<8; c++) {
      strength[c][0] = 3;
      strength[c][1] = 2;
      strength[c][2] = 1;
      strength[c][9] = 1;
      strength[c][10] = 1;
    }
  }
  else
  {
    for (int r=0; r<=3; r++)
    {
      for(int c=0; c<8; c+=2)
      {
        strength[c][r]=( (rand() % 4) + 1);
        strength[c+1][r]=strength[c][r];
      }
    }
    for (int r=9; r<=10; r++)
    {
      for(int c=0; c<8; c+=2)
      {
        strength[c][r]=( (rand() % 3) + 1);
        strength[c+1][r]=strength[c][r];
      }
    }
  }

}												// end if initBaord()

void Board::displayBlocks(  )
{
  for(int i = 0; i < 15; i++) {
    for(int j = 0; j < 8; j++ ) {
      if( strength[j][i] > 0 ) {
        matrix.drawPixel(j, i, LED_ON);
      }
    }
  }
}

int Board::getBlockHits()
{
	return blockHits;
}

int Board::setBlockHits(int input)
{
	blockHits = input;
}

void Board::updateBall()
{
  if(ballDown){
    if(ballRight){
      ballRow++;
      ballCol++;
    }
    else if(ballLeft){
      ballRow++;
      ballCol--;
    }
    else {
      ballRow++;
    }
  }
  else{
    if(ballRight){
      ballRow--;
      ballCol++;
    }
    else if (ballLeft) {
      ballRow--;
      ballCol--;
    }
    else {
      ballRow--;
    }
  }

  // Hack to ensure that the ball does not leave the field!
  if(ballCol < 0) {
    ballCol = 0;
  }
  else if(ballCol > 7) {
    ballCol = 7;
  }
  if(ballRow < 0) {
    ballRow = 0;
  }
  else if(ballRow > 15) {
    ballRow = 15;
  }

}

boolean Board::levelComplete(){
  for (int r=0; r<16; r++){
    for(int c=0; c<8; c++){
      if(!(strength[c][r]<=0)){
        return false;
      }
    }
  }
  return true;
}

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void gameOver(){
  
int gameTime = ((millis() / 1000) - 11);
int score = ((board.getBlockHits()*100) - gameTime);
  printMessage("Game Over");
  delay(2000);
  lcd.clear();
  printMessage("Final Score: ");
  lcd.setCursor(0,2);
  printMessage(score);
  delay(8000);
  resetFunc();
}

float smooth(int t_rawVal, float t_smoothedVal) {
    return  t_smoothedVal + ((t_rawVal - t_smoothedVal) + 0.5) / smoothStrength;  // +0.5 for rounding
}

void printMessage(int number) {
	lcd.print(number); // Print intro message to the LCD.
	delay(100); //set to 100
  return;
}

void printMessage(String message) {
    lcd.clear();
	lcd.print(message); // Print intro message to the LCD.
	delay(100); //set to 100
  return;
}
