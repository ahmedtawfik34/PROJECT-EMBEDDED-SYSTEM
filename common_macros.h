
#ifndef COMMON_MACROS
#define COMMON_MACROS

#define SET_BIT(REG,BIT) (REG |= (1<< BIT))

#define CLEAR_BIT(REG,BIT) (REG &=~ (1<< BIT))

#define TOGGLE_BIT(REG,BIT) (REG ^= (1<<BIT))

#define GET_BIT(REG,BIT) ((REG>>BIT) &0X01)
 
#define BIT_IS_SET(REG,BIT) (REG &(1<<BIT)) //((REG>>BIT) &1)  =1

#define BIT_IS_CLEAR(REG,BIT) (!(REG &(1<<BIT))) 

#endif 