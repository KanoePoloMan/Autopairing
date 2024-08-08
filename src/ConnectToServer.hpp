#ifndef __CONNECT_TO_SERVER_HPP__
#define __CONNECT_TO_SERVER_HPP__

#include "main.hpp"
#include "Autopairing.hpp"

void checkCommands(int sym);

void getMacAddrSerial();

void WifiCon();

void readNameAndPassSerial(char *name, char *pass);

void connectToWifi(char *name, char *pass);

#endif