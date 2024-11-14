#include "utils.h"
char *letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWZ0123456789!$";
String genreateRandomString(int len)
{
  String randomStr = "";
  for (int i = 0; i < len; i++)
  {
    randomStr = randomStr + letters[random(0, 63)];
  }
  return randomStr;
}