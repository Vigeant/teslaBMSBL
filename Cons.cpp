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

bool Cons::getCommandLineFromSerialPort(char* commandLine) {
  static uint8_t charsRead = 0;  //note: COMAND_BUFFER_LENGTH must be less than 255 chars long
  //read asynchronously until full command input
  while (Serial.available()) {
    //Serial.println("Available");
    char c = Serial.read();
    //Serial.printf("Received %d\n", c);
    switch (c) {
      case CR:  //likely have full command in buffer now, commands are terminated by CR and/or LF
      case LF:
        commandLine[charsRead] = NULLCHAR;  //null terminate our command char array
        if (charsRead >= 0) {
          charsRead = 0;  //charsRead is static, so have to reset
          //Serial.println(commandLine);
          Serial.println("");
          return true;
        }
        break;
      case BS:  // handle backspace in input: put a space in last char
      case 0x7f:
        if (charsRead > 0) {  //and adjust commandLine and charsRead
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
        commandLine[charsRead] = NULLCHAR;  //just in case
        break;
    }
  }
  return false;
}

void Cons::doConsole() {
  char* ptrToCommandName;
  if (getCommandLineFromSerialPort(cmdLine)) {
    ptrToCommandName = strtok(cmdLine, delimiters);
    if (ptrToCommandName != NULL) {
      auto i = cliCommands.begin();
      for (; i != cliCommands.end(); i++) {
        if (strcmp(ptrToCommandName, (*i)->tokenLong) == 0 || strcmp(ptrToCommandName, (*i)->tokenShort) == 0) {
          if ((*i)->doCommand() != 0) {
            Serial.printf("  Command failed: %s\n", ptrToCommandName);
          }
          break;
        }
      }
      if (i == cliCommands.end()) {
        Serial.printf("  Command not found: %s\n", ptrToCommandName);
      }
    }
    Serial.print("BMS> ");
  }
}

/////////////////////////////////////////////////
/// \brief Constructor
/////////////////////////////////////////////////
Cons::Cons(Controller* cont_inst_ptr)
  : commandPrintMenu(&cliCommands),
    showConfig(cont_inst_ptr->getSettingsPtr()),
    setParam(cont_inst_ptr),
    setDateTime(),
    showStatus(cont_inst_ptr),
    showGraph(cont_inst_ptr),
    showCSV(cont_inst_ptr),
    resetDefaultValues(cont_inst_ptr->getSettingsPtr()) {
  // initialize serial communication at 115200 bits per second:
  SERIALCONSOLE.begin(115200);
  SERIALCONSOLE.setTimeout(15);
  controller_inst_ptr = cont_inst_ptr;
  cliCommands.push_back(&commandPrintMenu);
  cliCommands.push_back(&showConfig);
  cliCommands.push_back(&resetDefaultValues);
  cliCommands.push_back(&setParam);
  cliCommands.push_back(&setDateTime);
  cliCommands.push_back(&showStatus);
  cliCommands.push_back(&showGraph);
  cliCommands.push_back(&showCSV);
}