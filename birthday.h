#include "config.h"
/**
 * Define a struct objec to hold birthday data
 */
struct Birthday{
  Birthday(){};
  Birthday(byte m, byte d, int y, String n){
    month = m;
    day = d;
    year = y;
    name = n;
  }
  byte month=0;
  byte day=0;
  int year=0;
  String name = "";

  bool isToday(byte m, byte d){
    return (month == m) && (day == d);
  }

  byte getAge(int curYear){
    return curYear - year;
  }  
};

Birthday birthday1 =  Birthday(BIRTHDAY_1_MONTH,BIRTHDAY_1_DAY,BIRTHDAY_1_YEAR,BIRTHDAY_1_Name);
Birthday birthday2 =  Birthday(BIRTHDAY_2_MONTH,BIRTHDAY_2_DAY,BIRTHDAY_2_YEAR,BIRTHDAY_2_Name);
Birthday birthday3 =  Birthday(BIRTHDAY_3_MONTH,BIRTHDAY_3_DAY,BIRTHDAY_3_YEAR,BIRTHDAY_3_Name);
Birthday birthday4 =  Birthday(BIRTHDAY_4_MONTH,BIRTHDAY_4_DAY,BIRTHDAY_4_YEAR,BIRTHDAY_4_Name);


/**
 * Check if there are any active birthdays today
 * and return the first one found
 */
Birthday* getBirthday(byte month, byte day){
  if (birthday1.isToday(month,day)){
    return &birthday1;
  }
  else if(birthday2.isToday(month,day)){
    return &birthday2;
  }
  else if(birthday3.isToday(month,day)){
    return &birthday3;
  }
  else if(birthday4.isToday(month,day)){
    return &birthday4;
  }
  else{
    return nullptr;
  }
}
