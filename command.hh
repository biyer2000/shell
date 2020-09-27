#ifndef command_hh
#define command_hh

#include "simpleCommand.hh"

// Command Data Structure

struct Command {
  std::vector<SimpleCommand *> _simpleCommands;
  std::string * _outFile;
  std::string * _inFile;
  std::string * _errFile;
  bool _background;
  bool _append;
  bool _valid;

  Command();
  void insertSimpleCommand( SimpleCommand * simpleCommand );
  void it();

  void clear();
  void prints();
  void execute();

  static SimpleCommand *_currentSimpleCommand;
};

#endif
