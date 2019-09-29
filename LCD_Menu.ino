#include <LiquidCrystal.h>
#include <rotary.h>                 // rotary handler

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//Define rotary encoder pins
#define PINA A0
#define PINB A1
#define PUSHB A2

//Define Main menu and sub menus...
#define MainMenu 0    //MainMenu
#define SubMenu1 1    //GameMenu
#define SubMenu2 2    //DifficultyMenu
#define SubMenu3 3    //LevelMenu
#define SubMenu4 4    //LightMenu
#define SubMenu5 5    //There is no SubMenu5...it is used for volume
#define SubMenu6 6    //LanguageMenu
#define SubMenu7 7    //Summary

const int maxItemSize = 11;  //longest menu item..includes char \0

// Initialize the Rotary object
// Rotary(Encoder Pin 1, Encoder Pin 2, Button Pin) Attach center to ground
Rotary r = Rotary(PINA, PINB, PUSHB);        // there is no must for using interrupt pins !!

int cursorLine = 1;
int displayFirstLine = 1;

//Define Menu Arrays
char startMenu[][maxItemSize] = {"Game", "Difficulty", "Level", "Light", "Volume", "Language", "Reset", "Summary"};
char One[][maxItemSize] = {"Pac Man", "Asteroids", "Centipede", "Invaders", "Cards", "Return"};
char Two[][maxItemSize] = {"Easy", "Medium", "Hard", "Return"};
char Three[][maxItemSize] = {"One", "Two", "Three", "Four", "Return"};
char Four[][maxItemSize] = {"ON", "OFF", "Return"};
//No Array needed for volume
char Six[][maxItemSize] = {"English", "Spanish", "Return"};

int Summary[] = {0, 0, 0, 0, 0, 0, 0, 0};   //store selected SubMenu

const int itemsPerScreen = 3;  // one less than max rows..
int menuItems;
char *menuSelected = NULL;
bool volumeHandler = false;
int volume = 0;
int menuOption = 0;

int backlightPin = 10;   //PWM pin
int brightness = 255;
int fadeAmount = 5;

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 10000;  //the value is a number of milliseconds

void setup ()
{
  digitalWrite (PINA, HIGH);     // enable pull-ups
  digitalWrite (PINB, HIGH);
  digitalWrite (PUSHB, HIGH);

  pinMode(backlightPin, OUTPUT);          //backlightPin as an output
  digitalWrite(backlightPin, HIGH);

  Serial.begin(9600);
  lcd.begin (20, 4);
  welcome();
  lcd.clear();

  //menuItems = sizeof(startMenu) / sizeof(startMenu[0]);
  menuItems = sizeof startMenu / sizeof * startMenu;
  menuSelected = &startMenu[0][0]; //Start Menu
  menuOption = MainMenu; //Main Menu

  startMillis = millis();  //initial start time
  display_menu(menuSelected, menuItems, maxItemSize);
}//end of setup

void loop ()
{
  volatile unsigned char result = r.process();

  currentMillis = millis();     //get the number of milliseconds since the program started
  lcd.setCursor(18, 0);         //(col, row)
  lcd.print((currentMillis - startMillis) / 1000);
  if (currentMillis - startMillis >= period)  //test whether the period has elapsed
  {
    LCDfadeOut();      //set LCD to sleep...
  } //End if currenMillis...

  if (result) {
    init_backlight();  //wake up LCD...
    if (result == DIR_CCW) {
      if (volumeHandler == true) {      //only used to increase or decrease volume
        volume--;
        lcd.setCursor(0, 3);
        lcd.print(volume);
      } else {
        move_up();
        chooseMenu();
      }
    } else {
      if (volumeHandler == true) {     //only used to increase or decrease volume
        volume++;
        lcd.setCursor(0, 3);
        lcd.print(volume);
      } else {
        move_down();
        chooseMenu();
      }
    }//end else
  }//end if Result

  if (r.buttonPressedReleased(25)) {
    init_backlight();  //wake up LCD...
    if (menuOption == MainMenu) {
      selectionMainMenu();
    }
    else if (menuOption == SubMenu7)
    {
      returnToMainMenu();
    } else {
      selectionSubMenu();
    }//end if menuSelected

    if (volumeHandler == false) {
      chooseMenu();
    }//end if volumeHandler
  }//endif buttonPressedReleased
}//end loop()

/**********************FUNCTIONS*************************/

void welcome()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Demo Menu  ");
  lcd.setCursor(0, 2);
  lcd.print("   Carlos Siles  ");
  lcd.setCursor(0, 3);
  lcd.print("   Version 1.0  ");
  delay(2000);
}//end welcome

void display_menu(const char *menuInput, int ROWS, int COLS)
{
  int n = 4;     //4 rows
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Menu  ");

  if (ROWS < n - 1) {
    n = ROWS + 1;
  }

  for (int i = 0; i < n - 1; i++) {
    lcd.setCursor(1, i + 1); //(col, row)
    for (int j = 0; j < COLS; j++) {
      if (*(menuInput + ((displayFirstLine + i - 1) * COLS + j)) != '\0') {
        lcd.print(*(menuInput + ((displayFirstLine + i - 1) * COLS + j)));
      }//end if
    }//end for j
  }//end for i

  lcd.setCursor(0, (cursorLine - displayFirstLine) + 1);
  lcd.print("<");
}//end display_menu

void move_down()
{
  if (cursorLine == (displayFirstLine + itemsPerScreen - 1)) {
    displayFirstLine++;
  }
  //If reached last item...roll over to first item
  if (cursorLine == menuItems) {
    cursorLine = 1;
    displayFirstLine = 1;
  } else {
    cursorLine = cursorLine + 1;
  }
}//end move_down

void move_up()
{
  if ((displayFirstLine == 1) & (cursorLine == 1)) {
    if (menuItems > itemsPerScreen - 1) {
      displayFirstLine = menuItems - itemsPerScreen + 1;
    }
  } else if (displayFirstLine == cursorLine) {
    displayFirstLine--;
  }

  if (cursorLine == 1) {
    if (menuItems > itemsPerScreen - 1) {
      cursorLine = menuItems; //roll over to last item
    }
  } else {
    cursorLine = cursorLine - 1;
  }
}//end move_up

void chooseMenu()
{
  // Check in which menu we are
  if (menuOption == SubMenu7)
  {
    display_summary();
  } else {
    display_menu(menuSelected, menuItems, maxItemSize);
  }
}//end chooseMenu

void selectionMainMenu()
{
  //volumeHandler activates volume to decrease/increase amount
  if (volumeHandler == true) {
    Summary[4] = volume;     //Sets the volume...
    volumeHandler = false;
    volume = 0;
    returnToMainMenu();
  } else {

    lcd.clear();

    switch (cursorLine - 1)
    {
      case 0:
        displayFirstLine = 1;       //initialize display_menu to 1st line
        cursorLine = 1;             //initialize display_menu to 1st line
        menuItems = sizeof One / sizeof * One;
        menuSelected = &One[0][0];
        menuOption = SubMenu1;
        break;
      case 1:
        displayFirstLine = 1;       //initialize display_menu to 1st line
        cursorLine = 1;             //initialize display_menu to 1st line
        menuItems = sizeof Two / sizeof * Two;
        menuSelected = &Two[0][0];
        menuOption = SubMenu2;
        break;
      case 2:
        displayFirstLine = 1;       //initialize display_menu to 1st line
        cursorLine = 1;             //initialize display_menu to 1st line
        menuItems = sizeof Three / sizeof * Three;
        menuSelected = &Three[0][0];
        menuOption = SubMenu3;
        break;
      case 3:
        displayFirstLine = 1;       //initialize display_menu to 1st line
        cursorLine = 1;             //initialize display_menu to 1st line
        menuItems = sizeof Four / sizeof * Four;
        menuSelected = &Four[0][0];
        menuOption = SubMenu4;
        break;
      case 4:
        displayFirstLine = 1;       //initialize display_menu to 1st line
        cursorLine = 1;             //initialize display_menu to 1st line
        lcd.setCursor(0, 0);
        lcd.print("Please set the ");
        lcd.setCursor(0, 1);
        lcd.print("Volume and click");
        volumeHandler = true;
        volume = Summary[4];
        lcd.setCursor(0, 3);
        lcd.print(volume);
        break;
      case 5:
        displayFirstLine = 1;       //initialize display_menu to 1st line
        cursorLine = 1;             //initialize display_menu to 1st line
        menuItems = sizeof Six / sizeof * Six;
        menuSelected = &Six[0][0];
        menuOption = SubMenu6;
        break;
      case 6:
        displayFirstLine = 1;       //initialize display_menu to 1st line
        cursorLine = 1;             //initialize display_menu to 1st line
        Reset();
        returnToMainMenu();
        break;
      case 7:
        displayFirstLine = 1;       //initialize display_menu to 1st line
        cursorLine = 1;             //initialize display_menu to 1st line
        menuItems = sizeof startMenu / sizeof * startMenu;
        menuOption = SubMenu7;
        break;
    }//end switch
  }//end else
}//end selectionMainMenu

void selectionSubMenu()
{
  lcd.clear();
  switch (cursorLine - 1)
  {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("option 1");
      Summary[menuOption - 1] = 1;  //user input saved in Summary array..to be used later
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("option 2");
      Summary[menuOption - 1] = 2;  //user input saved in Summary array..to be used later
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("option 3");
      if (menuOption != SubMenu4 && menuOption != SubMenu6)   //this is where return option is in the refered SubMenu
        Summary[menuOption - 1] = 3;  //user input saved in Summary array..to be used later
      break;
    case 3:
      lcd.setCursor(0, 0);
      lcd.print("option 4");
      if (menuOption != SubMenu2)     //this is where return option is in the refered SubMenu
        Summary[menuOption - 1] = 4;  //user input saved in Summary array..to be used later
      break;
    case 4:
      lcd.setCursor(0, 0);
      lcd.print("option 5");
      if (menuOption != SubMenu3)     //this is where return option is in the refered SubMenu
        Summary[menuOption - 1] = 5;  //user input saved in Summary array..to be used later
      break;
    case 5:
      lcd.setCursor(0, 0);
      lcd.print("option 6");
      if (menuOption != SubMenu1)     //this is where return option is in the refered SubMenu
        Summary[menuOption - 1] = 6;  //user input saved in Summary array..to be used later
      break;
  }//end switch
  delay(800);
  returnToMainMenu();
}//end selectionSubMenu

void Reset()
{
  for (int i = 0; i < sizeof(startMenu) / maxItemSize + 1; i++)
  {
    Summary[i] = 0;
  }
}//end Reset

void display_summary()
{
  int n = 4;      // 4 rows in LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Menu Summary  ");

  if (menuItems < n - 1) {
    n = menuItems + 1;
  }

  for (int i = 1; i < n; i++)
  {
    lcd.setCursor(1, i);   //(col, row)
    lcd.print(startMenu[displayFirstLine + i - 2]);
    lcd.setCursor(15, i);
    lcd.print(Summary[displayFirstLine + i - 2]);
  }
  lcd.setCursor(0, (cursorLine - displayFirstLine) + 1);
  lcd.print("<");
}//end display_summary

void returnToMainMenu()
{
  displayFirstLine = 1;
  cursorLine = 1;
  menuItems = sizeof startMenu / sizeof * startMenu;
  menuSelected = &startMenu[0][0];
  menuOption = MainMenu;
}//end returnToMainMenu

void LCDfadeOut()
{
  while (brightness > 0) {
    analogWrite(backlightPin, brightness);
    brightness -= fadeAmount;
    delay(20);
  }//end while
  digitalWrite(backlightPin, LOW);
  lcd.clear();
}//end LCDfadeOut

void init_backlight()
{
  digitalWrite(backlightPin, HIGH);
  startMillis = millis();  //initial start time
  brightness = 255;   //reset to initial brightness
}//end init_backlight
