/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h> 
#include "FreeRTOS.h"


#include "math.h"

#define 	CY_RSLT_SUCCESS   ((cy_rslt_t)0x00000000U)

// number of clock cycle (in cycles) 
#define WAKE_UP_TIMEOUT = 300u // 5 min at 1Hz
int NB_PACKET_GO_SLEEP = 10; // go to sleep  after 10 empty packets (of 1s)

typedef uint32_t 	cy_rslt_t;

bool switch2deepsleep = false;
int emptyPacketCounter  = 0;

int16_t bug = 4000;
int bugCounter = 0;
int debugCounter = 0;
bool ADCupdateFlag = false;


bool IOAmp = false; // GND first 


bool sharePastDrop = false;
uint32_t past_drop = 0;
uint8_t arrayPastDrop[4] = {0}; 


// enable set this to false to disable uart
#define uart_enabled 1

/**************************************
Function Prototypes
******************************/

// ADC Timer
void TimerInit(void);
void Isr_Timer(void);
// Wakeup timers
void Wakeup_TimerInit(void);
void WakeUp_interrupt (void);

//wake up interrupt 
void WakeupInterruptHandler(void);

// Callback Prototype
cy_en_syspm_status_t TCPWM_DeepSleepCallback(cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode);

int main(void)
{
    /* SysPm callback params */
	cy_stc_syspm_callback_params_t callbackParams = {
	    /*.base       =*/ NULL,
	    /*.context    =*/ NULL
	};
    
    cy_stc_syspm_callback_t PwmDeepSleepCb = {TCPWM_DeepSleepCallback,  /* Callback function */
                                              CY_SYSPM_DEEPSLEEP,       /* Callback type */
                                              CY_SYSPM_SKIP_CHECK_READY |
                                              CY_SYSPM_SKIP_CHECK_FAIL, /* Skip mode */
											  &callbackParams,       	/* Callback params */
                                              NULL, NULL};				/* For internal usage */
    
    
    __enable_irq(); /* Enable global interrupts. */

    TimerInit();
    Wakeup_TimerInit();
     /* Register SysPm callbacks */
    
    Cy_SysPm_RegisterCallback(&PwmDeepSleepCb);
    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    
    ADC_Start();
    Opamp_Start();
    Opamp2_Start();
    Opamp_Enable();
    
    Cy_SAR_StartConvert(SAR,CY_SAR_START_CONVERT_CONTINUOUS);
   
    #if (uart_enabled)
    UART_Start();
    #endif
    
    
    
    int16_t value = 0;
    int16_t volts = 0;
    char string[30];
    
    float tresh_up = 2000;     
    float tresh_down = 1000;

    int count_enable = 1;
    int total_drop = 0;   // total number of drops since program execution
    
    int packet_size = 48000;
    int packet_counter = 0;
    int current_drop =0;
   
    
    int divider =0 ; 
    int dividerFreq = 20;
    
    int16_t tempmax = 0; 
    int8_t tempmax2[packet_size*2];
    
    
    printf("Jean mmichel pif paf pouf\r\n");
    
    
   
    
    for(;;)
    {
       
        
    

        if(ADCupdateFlag){
            
            //ADC + conversion to volts
            value = Cy_SAR_GetResult16(SAR,0);
            volts = Cy_SAR_CountsTo_mVolts(SAR,0, value);
           
            
            #if (uart_enabled)
            if (volts > tempmax){
                   tempmax = volts;
                    }
            tempmax = -5;
            //tempmax2[packet_counter] = tempmax; 
            //tempmax2[packet_counter+1] = tempmax>>8;
            int N =2;
            if(!(divider < dividerFreq)){
            for( int i =0; i<N; ++i){
                    
                    //UART_Put(tempmax>>8*(i));
                    
                    
                }
            
            
            CyDelayUs(1);
            
            UART_Put(tempmax);
            CyDelayUs(1);
            
            UART_Put(tempmax>>8);
            //CyDelayUs(1);
            divider = 0;
            }
            else{
            divider++;
            }
            #endif
            
            //send voltage value through UART
            
            //sprintf(string,"%f\n",volts);
            //UART_PutString(string);
           
            //Calculating the number of raindrops 
            if (volts > tresh_up && count_enable == 1){
                
                count_enable = 0;
                total_drop ++;
                current_drop ++;
            }
            
            else if(volts < tresh_down){
                count_enable =1;
            }
            
            if ( packet_counter >= (packet_size - 1)){
                packet_counter = 0;
                past_drop = current_drop;
                current_drop =0;
                sharePastDrop = true;
                //Send number of drops in the last packet though UART
                //printf("past number of drops = \r");
                //sprintf(string,"%d, ",past_drop);
                #if 0
                for( int i =0; i<2; ++i){
                    
                    UART_Put(bug>>8*i);
                    
                }
                 for( int i =0; i<2; ++i){
                    
                    UART_Put(tempmax>>8*i);
                    
                }
                #endif
                tempmax = -5;
                #if uart_enabled 
                //sprintf(string,"%d\n",tempmax);
                //UART_PutArray(string);
                
                    
                uint16_t bytecount = 48000; 
                //UART_PutArray(&tempmax2, bytecount );
                    
                    
                
                #endif
                tempmax = 0 ;
                
                
                //while(Cy_IPC_Drv_SendMsgWord(IPC_STRUCT10,(1<<10),past_drop) != CY_IPC_DRV_SUCCESS ){
                //}
                
                // count the number of successive empty packet and set go2sleep to true after 10 empty packets
                if (past_drop == 0){
                    emptyPacketCounter ++;
                    if (emptyPacketCounter == NB_PACKET_GO_SLEEP){
                        //switch2deepsleep = true;
                        emptyPacketCounter = 0;
                    }
                }
                else{ emptyPacketCounter = 0;}
            }
            
            else {
                packet_counter ++;
            }
            
            ADCupdateFlag = false;
            debugCounter ++;
            
            
            if (debugCounter == 24000){
                //Cy_GPIO_Inv(LED_RED_PORT, LED_RED_NUM);
                debugCounter = 0;}
                }
            
        
        if (switch2deepsleep)
        {
            Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
        }
        if(sharePastDrop){
            //send past drop to CM0 and trigger an IPC interrupt 
            if(Cy_IPC_Drv_SendMsgWord(IPC_STRUCT10,(1<<10),past_drop) == CY_IPC_DRV_SUCCESS ){
                sharePastDrop = false;
            }
        }
             
        
    }
}


void Isr_Timer(void)
{
    /* Clear the TCPWM peripheral interrupt */
    Cy_TCPWM_ClearInterrupt(Timer_HW, Timer_CNT_NUM, CY_TCPWM_INT_ON_TC );
    
    /* Clear the CM4 NVIC pending interrupt for TCPWM */
    NVIC_ClearPendingIRQ(Isr_Timer_cfg.intrSrc);
	//Cy_SCB_UART_PutString(UART_HW, "Interrupt!\r\n");
	ADCupdateFlag = true;
}

void TimerInit(void)
{
    /* Configure the TCPWM peripheral.
       Counter_config structure is defined based on the parameters entered 
       in the Component configuration */
    Cy_TCPWM_Counter_Init(Timer_HW, Timer_CNT_NUM, &Timer_config);
    
    /* Enable the initialized counter */
    Cy_TCPWM_Counter_Enable(Timer_HW, Timer_CNT_NUM);
    
    /* Start the enabled counter */
    Cy_TCPWM_TriggerStart(Timer_HW, Timer_CNT_MASK);
    
    /* Configure the ISR for the TCPWM peripheral*/
    Cy_SysInt_Init(&Isr_Timer_cfg, Isr_Timer);
    
    /* Enable interrupt in NVIC */   
    NVIC_EnableIRQ((IRQn_Type)Isr_Timer_cfg.intrSrc);
}


void WakeUp_interrupt (void)
{
    uint32_t cause;
    cause = Cy_MCWDT_GetInterruptStatusMasked(MCWDT_STRUCT0);
    /* Clear the  peripheral interrupt */
    Cy_MCWDT_ClearInterrupt(MCWDT_STRUCT0,cause);
    
	
	switch2deepsleep = false;
}

void Wakeup_TimerInit(void)
{
    /* Configure the TCPWM peripheral.
       Counter_config structure is defined based on the parameters entered 
       in the Component configuration */
    
    
    MCWDT_1_Start();
    
    /* Configure the ISR for the TCPWM peripheral*/
    Cy_SysInt_Init(&WakeUp_interrupt_cfg, WakeUp_interrupt);
    
    /* Enable interrupt in NVIC */   
    NVIC_EnableIRQ((IRQn_Type)WakeUp_interrupt_cfg.intrSrc);
}

/*******************************************************************************
* Function Name: TCPWM_DeepSleepCallback
****************************************************************************//**
*
* Deep Sleep callback implementation. It turns the LED off before going to deep
* sleep power mode. After waking up, it sets the LED to blink.
* Note that the PWM block needs to be re-enabled after waking up, since the
* clock feeding the PWM is disabled in deep sleep.
*
*******************************************************************************/
cy_en_syspm_status_t TCPWM_DeepSleepCallback(
    cy_stc_syspm_callback_params_t *callbackParams, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_FAIL;

	switch (mode)
	{
		case CY_SYSPM_BEFORE_TRANSITION:
			/* Before going to sleep mode, turn off the LEDs */
            
			Cy_GPIO_Set(LED_RED_PORT, LED_RED_NUM);

            //stop feeding the amplifier
            
            
			/* Disable the ADC counter */
			Cy_TCPWM_Counter_Disable(Timer_HW, Timer_CNT_NUM);

			retVal = CY_SYSPM_SUCCESS;
			break;

		case CY_SYSPM_AFTER_TRANSITION:
            
            //feed the amplifier
            
    
			/* Re-enable Timer for ADC */
            
			TimerInit();

			retVal = CY_SYSPM_SUCCESS;
			break;

		default:
			/* Don't do anything in the other modes */
			retVal = CY_SYSPM_SUCCESS;
			break;
	}

    return retVal;
}
/* [] END OF FILE */
