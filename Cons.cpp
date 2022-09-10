#include "Cons.hpp"
#include "Config.hpp"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define CONSOLEBUFFERSIZE 64

#define CR '\r'
#define LF '\n'
#define BS '\b'
#define NULLCHAR '\0'
#define SPACE ' '

/////////////////////////////////////////////////
/// \brief This is the consoles main function where commands are interpreted every tick.
/////////////////////////////////////////////////

bool Cons::getCommandLineFromSerialPort(char * commandLine) {
  static uint8_t charsRead = 0;                      //note: COMAND_BUFFER_LENGTH must be less than 255 chars long
  //read asynchronously until full command input
  while (Serial.available()) {
    //Serial.println("Available");
    char c = Serial.read();
    //Serial.printf("Received %d\n", c);
    switch (c) {
      case CR:      //likely have full command in buffer now, commands are terminated by CR and/or LF
      case LF:
        commandLine[charsRead] = NULLCHAR;       //null terminate our command char array
        if (charsRead >= 0)  {
          charsRead = 0;                           //charsRead is static, so have to reset
          //Serial.println(commandLine);
          Serial.println("");
          return true;
        }
        break;
      case BS:// handle backspace in input: put a space in last char
      case 0x7f:
        if (charsRead > 0) {                        //and adjust commandLine and charsRead
          commandLine[--charsRead] = NULLCHAR;
          Serial.print("\b \b");
        }
        break;
      default:
        // c = tolower(c);
        if (charsRead < COMMAND_BUFFER_LENGTH) {
          commandLine[charsRead++] = c;
          Serial.print(commandLine[charsRead - 1]);
        }
        commandLine[charsRead] = NULLCHAR;     //just in case
        break;
    }
  }
  return false;
}

void Cons::doConsole() {
  uint32_t i;
  char * ptrToCommandName;
  //Serial.println("do Console");
  if ( getCommandLineFromSerialPort(cmdLine) ) {
    //Serial.println("got something");
    ptrToCommandName = strtok(cmdLine, delimiters);
    for (i = 0; i < NUMBER_OF_COMMANDS; i++) {
      //Serial.println(cliCommands[i]->tokenLong);
      //Serial.flush();
      if (strcmp(ptrToCommandName, cliCommands[i]->tokenLong) == 0 || strcmp(ptrToCommandName, cliCommands[i]->tokenShort) == 0) {
        if (cliCommands[i]->doCommand() != 0) {
          Serial.printf("  Command failed: %s\n", ptrToCommandName);
        }
        break;
      }
    }
    if (cliCommands[i] == 0) {
      Serial.printf("  Command not found: %s\n", ptrToCommandName);
    }
    Serial.print("BMS> ");
  }
}

/////////////////////////////////////////////////
/// \brief Constructor
/////////////////////////////////////////////////
Cons::Cons(Controller* cont_inst_ptr) {
  // initialize serial communication at 115200 bits per second:
  SERIALCONSOLE.begin(115200);
  SERIALCONSOLE.setTimeout(15);
  controller_inst_ptr = cont_inst_ptr;
  uint32_t i = 0;
  if (i < NUMBER_OF_COMMANDS) {cliCommands[i++] = new CommandPrintMenu(cliCommands);}
  if (i < NUMBER_OF_COMMANDS) {cliCommands[i++] = new ShowStatus(controller_inst_ptr);}
  if (i < NUMBER_OF_COMMANDS) {cliCommands[i++] = new ShowConfig(controller_inst_ptr->getSettingsPtr());}
  if (i < NUMBER_OF_COMMANDS) {cliCommands[i++] = new ShowGraph(controller_inst_ptr);}
  if (i < NUMBER_OF_COMMANDS) {cliCommands[i++] = new ShowCSV(controller_inst_ptr);}
  if (i < NUMBER_OF_COMMANDS) {cliCommands[i++] = new SetVerbose();}
}
