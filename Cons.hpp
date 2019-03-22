#include "Logger.hpp"
#include "Controller.hpp"

class Cons {
  public:
    Cons(Controller*);
    void doConsole();
    void printMenu();
  private:
    Controller* controller_inst_ptr;
};
