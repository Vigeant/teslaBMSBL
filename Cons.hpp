#include "Logger.hpp"
#include "Controller.hpp"
#include <string.h>
#include <list>
//#include <vector>



class CliCommand {
  public:
    virtual int doCommand() = 0;
    const char* name;
    const char* tokenLong;
    const char* tokenShort;
    const char* help;
  protected:
    //CliCommand** cliCommands;
    Controller* controller_inst_ptr;
};

class CommandPrintMenu : public CliCommand {
  public:
    CommandPrintMenu(std::list<CliCommand*> *cliComm) {
      name = "Help";
      tokenLong = "help";
      tokenShort = "h";
      help = " | show this help";
      cliCommands = cliComm;
    }
    int doCommand() {
      //uint32_t i;
      uint32_t printed;
      Serial.print("\n\\||||||||/   \\||||||||/   |||||||||/   ||           \\||||||||/\n");
      Serial.print("    ||                    ||           ||\n");
      Serial.print("    ||       \\||||||||/   ||||||||||   ||           ||||||||||\n");
      Serial.print("    ||                            ||   ||           ||      ||\n");
      Serial.print("    ||       \\||||||||/   /|||||||||   |||||||||/   ||      ||\n\n");
      Serial.print("              Battery Management System by GuilT\n");
      Serial.print("\n************************* SYSTEM MENU *************************\n");
      Serial.print("GENERAL SYSTEM CONFIGURATION\n\n");


      for (auto i = (*cliCommands).begin(); i != (*cliCommands).end() ; i++) {
        printed = strlen((*i)->tokenShort) + strlen((*i)->tokenLong);

        /*
              }
              for (i = 0; i < Cons::NUMBER_OF_COMMANDS; i++) {
                printed = strlen(cliCommands[i]->tokenShort) + strlen(cliCommands[i]->tokenLong);*/
        if (printed >= 18) {
          printed = 17;
        }

        Serial.print((*i)->tokenShort);
        Serial.print(" or ");
        Serial.print((*i)->tokenLong);
        for (; printed < 18; printed++) {
          Serial.print(" ");
        }
        Serial.print((*i)->help);
        Serial.print("\n");
      }
      Serial.print("\n");
      return 0;
    }
  private:
    std::list<CliCommand*> *cliCommands;
};

class ShowConfig : public CliCommand {
  public:
    ShowConfig(Settings* sett) {
      name = "Show Config";
      tokenLong = "config";
      tokenShort = "c";
      help = " | show all configuration settings";
      settings = sett;
    }
    int doCommand() {
      settings->printSettings();
      return 0;
    }
  private:
    Settings* settings;
};

class SetParam : public CliCommand {
  public:
    SetParam(Controller* cont_inst_ptr) {
      name = "Set Param";
      tokenLong = "set";
      tokenShort = "s";
      help = " | set a config parameter to a specific value";
      controller_inst_ptr = cont_inst_ptr;
      settings = cont_inst_ptr->getSettingsPtr();
    }
    int doCommand() {
      char* paramName, *valStr;
      Param* param;
      paramName = strtok(0, 0);
      if (paramName == 0) {
        return -1;
      } else {
        paramName = strtok(paramName, " ");
        param = settings->getParam(paramName);
        if (param == 0) {
          return -2;
        } else {
          valStr = strtok(0, 0);
          if (valStr == 0) {
            return -3;
          } else {
            if (param->setVal(valStr) == 0) {
              if (controller_inst_ptr->saveSettings() == 0){
                return 0;
              } else {
                return -5;
              }
            } else {
              return -4;
            }
          }
        }
      }
      return -6;
    }
  private:
    Settings* settings;
};

class ShowStatus : public CliCommand {
  public:
    ShowStatus(Controller* cont_inst_ptr) {
      name = "Show Status";
      tokenLong = "status";
      tokenShort = "1";
      help = " | shortcut to show BMS status summary";
      controller_inst_ptr = cont_inst_ptr;
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
    ShowGraph(Controller* cont_inst_ptr) {
      name = "Show Graph";
      tokenLong = "graph";
      tokenShort = "2";
      help = " | shortcut to show cell volatage graph";
      controller_inst_ptr = cont_inst_ptr;
    }
    int doCommand() {
      controller_inst_ptr->getBMSPtr()->printPackGraph();
      return 0;
    }
};

class ShowCSV : public CliCommand {
  public:
    ShowCSV(Controller* cont_inst_ptr) {
      name = "Show CSV";
      tokenLong = "CSV";
      tokenShort = "3";
      help = " | shortcut to show BMS details in CSV format";
      controller_inst_ptr = cont_inst_ptr;
    }
    int doCommand() {
      controller_inst_ptr->getBMSPtr()->printAllCSV();
      return 0;
    }
};

class ResetDefaultValues : public CliCommand {
  public:
    ResetDefaultValues(Settings* sett) {
      name = "reset default values";
      tokenLong = "resetDefaultValues";
      tokenShort = "reset";
      help = " | Reset al default configuration values";
      settings = sett;
    }
    int doCommand() {
      settings->reloadDefaultSettings();
      return 0;
    }
  private:
    Settings* settings;
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
    Cons(Controller* cont_inst_ptr);
    void doConsole();
    static const uint32_t NUMBER_OF_COMMANDS = 6;
    static const uint32_t COMMAND_BUFFER_LENGTH = 64;  //length of serial buffer for incoming commands


  private:
    char  cmdLine[COMMAND_BUFFER_LENGTH + 1];  //Read commands into this buffer from Serial.  +1 in length for a termination char
    CommandPrintMenu commandPrintMenu;
    ShowConfig showConfig;
    SetParam setParam;
    ShowStatus showStatus;
    ShowGraph showGraph;
    ShowCSV showCSV;
    SetVerbose setVerbose;
    ResetDefaultValues resetDefaultValues;
    std::list<CliCommand*> cliCommands;
    Controller* controller_inst_ptr;
    const char *delimiters            = ", \n";
    bool getCommandLineFromSerialPort(char * commandLine);
};
