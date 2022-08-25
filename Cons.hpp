#include "Logger.hpp"
#include "Controller.hpp"
#include <string.h>
//#include <vector>

class CliCommand {
  public:
    virtual int doCommand() = 0;
    const char* name;
    const char* tokenLong;
    const char* tokenShort;
    const char* help;
};

//class CommandPrintMenu;
extern CliCommand* cliCommands[];
static Controller* controller_inst_ptr;

class CommandPrintMenu : public CliCommand {
  public:
    CommandPrintMenu() {
      name = "Help";
      tokenLong = "help";
      tokenShort = "h";
      help = " | show this help";
    }
    int doCommand() {
      uint32_t i;
      uint32_t printed;
      Serial.print("\n\\||||||||/   \\||||||||/   |||||||||/   ||           \\||||||||/\n");
      Serial.print("    ||                    ||           ||\n");
      Serial.print("    ||       \\||||||||/   ||||||||||   ||           ||||||||||\n");
      Serial.print("    ||                            ||   ||           ||      ||\n");
      Serial.print("    ||       \\||||||||/   /|||||||||   |||||||||/   ||      ||\n\n");
      Serial.print("              Battery Management System by GuilT\n");
      Serial.print("\n************************* SYSTEM MENU *************************\n");
      Serial.print("GENERAL SYSTEM CONFIGURATION\n\n");

      for (i = 0; cliCommands[i] != 0; i++) {
        printed = strlen(cliCommands[i]->tokenShort) + strlen(cliCommands[i]->tokenLong);
        if (printed >= 18) {
          printed = 17;
        }

        Serial.print(cliCommands[i]->tokenShort);
        Serial.print(" or ");
        Serial.print(cliCommands[i]->tokenLong);
        for (; printed < 18; printed++) {
          Serial.print(" ");
        }
        Serial.print(cliCommands[i]->help);
        Serial.print("\n");
      }
      /*
        LOG_CONSOLE("   h or ? = help (displays this message)\n");
        LOG_CONSOLE("   1 = display BMS status summary\n");
        LOG_CONSOLE("   2 = print cell voltage graph\n");
        LOG_CONSOLE("   3 = output BMS details in CSV format\n");
        LOG_CONSOLE("   vX verbose (X=0:debug, X=1:info, X=2:warn, X=3:error, X=4:Cons)\n");*/
      Serial.print("\n");
      return 0;
    }
};

class ShowStatus : public CliCommand {
  public:
    ShowStatus() {
      name = "Show Status";
      tokenLong = "status";
      tokenShort = "1";
      help = " | shortcut to show BMS status summary";
    }
    int doCommand() {
      controller_inst_ptr->getBMSPtr()->printPackSummary();
      LOG_CONSOLE("12V Battery: %.2fV \n", controller_inst_ptr->bat12vVoltage);
      controller_inst_ptr->printControllerState();
      return 0;
    }
};

class ShowGraph : public CliCommand {
  public:
    ShowGraph() {
      name = "Show Graph";
      tokenLong = "graph";
      tokenShort = "2";
      help = " | shortcut to show cell volatage graph";
    }
    int doCommand() {
      controller_inst_ptr->getBMSPtr()->printPackGraph();
      return 0;
    }
};

class ShowCSV : public CliCommand {
  public:
    ShowCSV() {
      name = "Show CSV";
      tokenLong = "CSV";
      tokenShort = "3";
      help = " | shortcut to show BMS details in CSV format";
    }
    int doCommand() {
      controller_inst_ptr->getBMSPtr()->printAllCSV();
      return 0;
    }
};

class SetVerbose : public CliCommand {
  public:
    SetVerbose() {
      name = "Verbose";
      tokenLong = "verbose";
      tokenShort = "v";
      help = " | set verbosity eg.: v X (X=0:debug, X=1:info, X=2:warn, X=3:error, X=4:Cons)";
    }
    int doCommand() {
      char* verbosity;
      uint32_t verbo;
      verbosity = strtok(0, 0);
      if (verbosity != 0) {
        verbo = atoi(verbosity);
        if (verbo >= 0 && verbo <= 5) {
          log_inst.setLoglevel((Logger::LogLevel)(verbo));
          return 0;
        } else {
          Serial.print("logLevel out of bounds (0-5)\n");
          return 2;
        }
      }
      Serial.print("missing logLevel eg.: BMS> v 3\n");
      return 1;
    }
};

class Cons {
  public:
    Cons(Controller*);
    void doConsole();
  private:
    const char *delimiters            = ", \n";
};
