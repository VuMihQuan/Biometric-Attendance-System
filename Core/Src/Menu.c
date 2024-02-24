#include "Menu.h"

//void handleButton(){ 
//	if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == 0 || HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == 0 || HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)){
//		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == 0){
//			(menu == 1)? (menu = 3) : (menu--);
//			featureMenu(menu);
//		}
//		else if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == 0){
//			(menu == 3)? (menu = 1) : (menu++);
//			featureMenu(menu);
//		}
//		else if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == 0){
//			i
//		}
//	}
//}


void featureMenu(int m){
	if(m == 1){
			lcd_clear_display();
			HAL_Delay(100);
			setup_send("> Home", "Register");
	}
	else if(m == 2){
			lcd_clear_display();
			HAL_Delay(100);
			setup_send("Home", "> Register");
	}
	else if(m == 3){
			lcd_clear_display();
			HAL_Delay(100);
			setup_send("Register", "> Delete");
	}
}


