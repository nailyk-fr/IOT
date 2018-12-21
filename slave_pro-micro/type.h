// Customs type for RF transmission: 
typedef union
{
  float value;
  char bytes[5];
} RFFLOAT;

struct RFDATA {
    char addr;
    RFFLOAT rffloat;
} ; 

