//Librarys
#include <Servo.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

/* 

Secret Book Safe
Made by: Skylynn Cantor 
Created for: WREBOTICS

FINAL EDIT 10/10/2024

Notes: 
- EEPROM is non-volatile memory that retains
data even when the power is turned off. In the context of this code, 
it is used to store custom user input password when safe is powered 
off or loses power and ensures that the custom password will retain

 */

//initializing servo object (latch that locks safe)
Servo latch;

//LCD screen initialization
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Defines password length
#define Pass_length 7
// Starting address for password storage
#define EEPROM_ADDRESS 0


/*Declares an array of characters, temporatily stores the password digits
that the user inputs through the keypad */
char Data[Pass_length];
char Master[Pass_length] = "333333";  //Default Password

//index that keeps track of how many keys have been entered
byte data_count = 0;

//initialize array of keypad
const byte ROWS = 4;
const byte COLS = 3;

// Define keymap
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

// Connect rows to pin count
byte rowPins[ROWS] = { 9, 8, 7, 6 };
byte colPins[COLS] = { 5, 4, 3 };

//initialize an instance of class NewKeypad
Keypad customKey = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

const int lockedPosition = 90;     // Position for locked state
const int unlockedPosition = -90;  // Position for unlocked state


bool firstRun = true;       //flag to check if its the first run of the loop
int incorrectAttempts = 0;  //counter for incorrect password attempts

//function is called when the password is correct and unlocks the safe
void unlock() {
  lcd.setCursor(0, 0);
  lcd.print("Correct!");
  lcd.setCursor(0, 1);
  lcd.print("Unlocking..");
  latch.write(-90);  //moves latch to unlock position
                     //stays locked until hash key is pressed
  delay(5000);       //wait for 5 sec
  clearData();       //clear input and data array after locking
  lcd.setCursor(0, 0);
  firstRun = true;
}

//if password is incorrect this will lock the box for two minutes
void lock() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lock Disabled");
  lcd.setCursor(0, 1);
  lcd.print("2min remaining");
  delay(2000);
  lcd.clear();

  //countdown from 120 seconds
  for (int i = 120; i >= 0; i--) {
    lcd.setCursor(0, 0);
    lcd.print("time remaining:");
    lcd.setCursor(0, 1);  // Set cursor to second line
    lcd.print(i);         // Print the current countdown value
    lcd.print(" sec  ");

    delay(1000);  // Wait for 1 second

    if (i == 0) {   //when countdown reaches zero
      lcd.clear();  // Clear the LCD
      firstRun = true;
    }
  }
}

void loadPassword() {
  for (int i = 0; i < Pass_length; i++) {         // Loop through each character in the password
    Master[i] = EEPROM.read(EEPROM_ADDRESS + i);  // Read the character from EEPROM
  }
  Master[Pass_length - 1] = '\0';  // Ensure null termination
}


// Function to save the password to EEPROM
void savePassword() {
  for (int i = 0; i < Pass_length; i++) {         // Loop through each character
    EEPROM.write(EEPROM_ADDRESS + i, Master[i]);  // Write the character to EEPROM
  }
}

void locking() {
  int currentPosition = latch.read();  // Read the current position of the latch

  //if already locked, dont lock
  if (currentPosition == lockedPosition) {
    lcd.clear();
    lcd.print("Already Locked");
    delay(3000);
    lcd.clear();
  }

  //checks if already locked, if not locked, then lock
  if (currentPosition != lockedPosition) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Locking...");
    latch.write(lockedPosition);  //move servo latch into locked position
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Locked");
    delay(2000);
    clearData();
  }
  data_count = 0;
  firstRun = true;
}

//resets the input data and clears the LCD Display
void clearData() {
  //this will keep running as long as there are in elements in the Data array to clear
  while (data_count != 0) {
    //clears the entry
    Data[data_count--] = 0;
  }
  lcd.clear();
}

//function to change password
void changePassword() {
  clearData();
  lcd.setCursor(0, 0);
  lcd.print("Changing Pass");
  lcd.setCursor(0, 1);
  lcd.print("0's to Cancel");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter new Pass:");

  data_count = 0;  // Reset data count for new password input

  // Wait for user to input a new password
  while (data_count < Pass_length - 1) {  // Loop until password length is reached
    char key = customKey.getKey();        // Get the pressed key

    if (key) {                             // Check if a key was pressed
      if (data_count < Pass_length - 1) {  // Check for available space
        Data[data_count++] = key;          // Store the key
        lcd.setCursor(data_count - 1, 1);  // Move to the next position
        lcd.print(key);                    // Display the key
      }
    }
  }

  Data[data_count] = '\0';  // Null terminate the string

  // Check if "000000" was typed to cancel change
  if (strcmp(Data, "000000") == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Change Cancelled");
    delay(2000);
    clearData();
    firstRun = true;
    return;  // Exit the function
  }

  // Store the new password temporarily for comparison
  char newPassword[Pass_length];  // Create an array to hold the new password
  strcpy(newPassword, Data);      // Copy the entered password to newPassword

  // Confirming the new password
  clearData();  // Clear input for confirmation
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirm Pass:");

  data_count = 0;  // Reset data count for confirmation input

  // Wait for user to input the confirmation password
  while (data_count < Pass_length - 1) {  // Loop until password length is reached
    char key = customKey.getKey();        // Get the pressed key

    if (key) {
      if (data_count < Pass_length - 1) {  // Check for available space
        Data[data_count++] = key;          // Store the key
        lcd.setCursor(data_count - 1, 1);  // Move to the next position
        lcd.print(key);                    // Display the key
      }
    }
  }

  Data[data_count] = '\0';  // Null terminate the string

  // Compare new password and confirmation
  if (strcmp(Data, newPassword) == 0) {  // Compare with the new password
    strcpy(Master, newPassword);         // Copy new password to Master
    savePassword();                      // Save to EEPROM
    lcd.clear();
    lcd.print("Password Changed");
    delay(2000);

  } else {
    lcd.clear();
    lcd.print("Passwords Don't");
    lcd.setCursor(0, 1);
    lcd.print("Match!");
    delay(2000);
  }

  clearData();      // Clear input data after processing
  firstRun = true;  // Reset for next input
}


void setup() {
  //set up serial comunication between lcd and arduino
  Serial.begin(9600);
  //initializes the LCD Screen
  lcd.init();
  //Turn on LCD Backlight
  lcd.backlight();
  //pin that attaches servo (latch) on arduino
  latch.attach(10);

  loadPassword();

  // Check if EEPROM is empty or uninitialized
  bool isInitialized = true;
  for (int i = 0; i < Pass_length; i++) {
    // Compare stored password with the default password
    if (EEPROM.read(EEPROM_ADDRESS + i) != Master[i]) {
      isInitialized = false;  // If any byte differs, mark as uninitialized
      break;                  // Exit the loop as we found a difference
    }
  }

  // If not initialized, write the default password
  if (!isInitialized) {
    savePassword();  // Save default password to EEPROM
  }

  loadPassword();               // Load the password from EEPROM
  latch.write(lockedPosition);  // Start in locked position
}

void loop() {

  //Uncomment to check what password is stored in EEPROM
  // if (!strcmp(Data, Master)) {
  //   Serial.println("Password matches!");
  //   unlock();  // Unlock if password is correct
  // } else {
  //   Serial.print("Entered: ");
  //   Serial.println(Data);
  //   Serial.print("Stored: ");
  //   Serial.println(Master);
  // }

  //main display
  /* if there is no firstRun check here, "Enter Password" 
  will override whatever else is being printed on the lcd screen */
  if (firstRun == true) {  // Check if it's the first run of the loop
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:");
    firstRun = false;  // Set firstRun to false to prevent repeated prompts
  }

  // Get the pressed key from the keypad
  char key = customKey.getKey();
  //if key is pressed
  if (key) {

    // prints each digit pressed
    Data[data_count] = key;        // Store the key in the Data array
    lcd.setCursor(data_count, 1);  // Move cursor to the next position in the second row
    lcd.print(Data[data_count]);   // Display the pressed key
    data_count++;                  // Increment the count of entered digits

    //check if enough characters have been entered
    if (data_count == Pass_length - 1) {
      Data[data_count] = '\0';  //Null terminate the string
      lcd.clear();
      //strcmp compares two arrays character by character
      // comparing if Data and Master are the same
      //if Data and Master are the exact same, strcmp will return 0 so !0 will make the statement true
      if (!strcmp(Data, Master)) {
        unlock();  //unlocks if password is correct
      }

      //if password inputted is incorrect
      else {
        incorrectAttempts++;  // Increment the number of incorrect attempts
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Error:");
        lcd.setCursor(0, 1);
        lcd.print("Wrong Password");
        delay(2000);
        firstRun = true;

        //checks if wrong password limit was exceeded
        if (incorrectAttempts >= 3) {
          lock();  // Lock the safe if there are too many incorrect attempts
        }
        clearData();  //clear the input and entire screen
      }
    }



    //if # is pressed, lock the safe
    if (key == '#') {
      locking();  //locks the safe
    }

    //checks for requirements to change password
    if (key == '*') {
      int currentPosition = latch.read();  // Read the current position of the latch
      firstRun = true;

      //if unlocked, proceed to change password
      if (currentPosition != lockedPosition) {
        changePassword();
      }

      //if locked, not able to change password
      if (currentPosition == lockedPosition) {
        //unable to change if lockec
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Unable to change");
        lcd.setCursor(0, 1);
        lcd.print("Password");
        delay(3000);
        clearData();
        firstRun = true;
      }
    }
  }
}
