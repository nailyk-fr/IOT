// Customs type for RF transmission: 
typedef union
{
  float value;
  char bytes[5];
} RFFLOAT;

typedef union
{
  char value;
  char bytes[1];  
} RFADDR;

struct RFDATA {
    RFADDR addr;
    RFFLOAT rffloat;
} ; 
