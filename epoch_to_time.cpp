#include "epoch_to_time.h"

void epoch2time(int* year_out,int* month_out,unsigned long* day_out, int* week, int* hour, int* minut, unsigned long epoch, int UTC){

  int year;
  bool y_flag;
  int month;
  unsigned long day;
  int day_2;
  epoch += (UTC*3600);
  *hour = (epoch  % 86400L) / 3600;
  *minut = (epoch % 3600) / 60; 
  *week = (epoch/86400+1)%7;


  day = epoch/86400+1;
  year = 1970;
  y_flag=1; 
  while(y_flag!=0){
    if((!(year%4))&&((year%100)||(!(year%400)))){
      if(day>366){
        day=day-366;
        year=year+1;
      }
      else{
        y_flag=0;
      }
    }
    else{
      if(day>365){
        day=day-365;
        year=year+1;
      }
      else{
        y_flag=0;
      }
    }      
    }
    
      
   
   if((!(year%4))&&((year%100)||(!(year%400)))){
    day_2=29;
   }
   else{
    day_2=28;
   }
    month=1;
      if(day>31){
        day=day-31;
        month=month+1;
        if(day>day_2){
          day=day-day_2;
          month=month+1;
          if(day>31){  //3
            day=day-31;
            month=month+1;
            if(day>30){
              day=day-30;
              month=month+1;
              if(day>31){
                day=day-31;
                month=month+1;
                if(day>30){
                  day=day-30;
                  month=month+1;
                  if(day>31){  //7
                    day=day-31;
                    month=month+1;
                    if(day>31){
                      day=day-31;
                      month=month+1;
                      if(day>30){
                        day=day-30;
                        month=month+1;
                        if(day>31){
                          day=day-31;
                          month=month+1;
                          if(day>30){
                            day=day-30;
                            month=month+1;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }


  *year_out = year;
  *month_out = month;
  *day_out = day;
}
