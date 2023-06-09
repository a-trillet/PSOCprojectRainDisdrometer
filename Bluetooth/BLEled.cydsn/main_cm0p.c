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
#include "FreeRTOS.h"

#define 	CY_RSLT_SUCCESS   ((cy_rslt_t)0x00000000U)

typedef uint32_t 	cy_rslt_t;

 #define MY_IPC_CHAN_INDEX       (10UL) /* Example of IPC channel index */
 #define MY_IPC_INTR_INDEX       (10UL) /* Example of IPC interrupt channel index */
 #define MY_IPC_INTR_MASK        (1UL << MY_IPC_INTR_INDEX) /* IPC release interrupt mask */
 #define MY_IPC_RELEASE_INTR_MASK (1UL << MY_IPC_CHAN_INDEX) /* IPC release interrupt mask */
 #define MY_IPC_ACQUIRE_INTR_MASK (1UL << MY_IPC_CHAN_INDEX) /* IPC acquire interrupt mask */

//drops per 10 sec
uint8_t counter = 0x00;
uint8_t oldCounter = 0x00;

uint8_t ledValue = 0x00;

float freq = 0.1;

int counterNotify;

int buttonReset = 0;

char BleConnected = 0;

void updateLEDon();
void toggleLedvalue();



/*****************************************************************************\
 * Function:    genericEventHandler(uint32_t event, void *eventParameter)
 * Input:       uint32_t event, void *eventParameter
 * Returns:     void
 * Description: 
 *   This function handles the BLE events. It powers the green LED when the BLE
 *   connection is active. It starts a BLE advertisement when disconnected.
 *   It handles the RED_LED_ON write request and the RED_LED_INV write request.
\*****************************************************************************/
void genericEventHandler(uint32_t event, void *eventParameter)
{
    cy_stc_ble_gatts_write_cmd_req_param_t   *writeReqParameter;
    
    /* Take an action based on the current event */
    switch (event)
    {
      
        /* This event is received when the BLE stack is Started */
        case CY_BLE_EVT_STACK_ON:
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
            Cy_GPIO_Set(LED_GREEN_PORT, LED_GREEN_NUM); //Turn off the LED_GREEN when disconnected
            Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            BleConnected = 0;
        break;

        case CY_BLE_EVT_GATT_CONNECT_IND: // Connection indication
            Cy_GPIO_Clr(LED_GREEN_PORT, LED_GREEN_NUM); //Turn on the LED_GREEN when connected
            BleConnected = 1;
        break;
        
        case CY_BLE_EVT_GATTS_WRITE_REQ: // When the other side sends the write request
            writeReqParameter = (cy_stc_ble_gatts_write_cmd_req_param_t *)eventParameter;
            
            /* Turns on the red LED when a value higher than 0 is sent through the BLE RED_LED_ON service */
            if(CY_BLE_DEV_INT_RED_LED_ON_CHAR_HANDLE == writeReqParameter->handleValPair.attrHandle)
            {              
                uint32_t val = writeReqParameter->handleValPair.value.val[0];
                ledValue = val;
                updateLEDon();
                if(val>0)
                {
                Cy_GPIO_Clr(LED_RED_PORT, LED_RED_NUM);
                
                }
                if(val == 0)
                {
                    Cy_GPIO_Set(LED_RED_PORT, LED_RED_NUM);
                }
            }
            
            /* Toggles red LED when value higher than one is sent to RED_LED_TOGGLE .*/
            if(CY_BLE_DEV_INT_RED_LED_TOGGLE_CHAR_HANDLE == writeReqParameter->handleValPair.attrHandle)
            {              
                uint32_t toggle = writeReqParameter->handleValPair.value.val[0];
                if(toggle>0)
                {
                toggleLedvalue();
                }
                
            }
            /* update the counter with value on write*/
            if(CY_BLE_DEV_INT_COUNTER_CHAR_HANDLE == writeReqParameter->handleValPair.attrHandle)
            {              
                uint32_t val = writeReqParameter->handleValPair.value.val[0];
                if(val>0)
                {
                counter = (uint8_t) &val;
                }
                
            }         
            #if 0
            if(CY_BLE_DEV_INT_COUNTER_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE == writeReqParameter->handleValPair.attrHandle){
                
            }
            #endif
            
            Cy_BLE_GATTS_WriteRsp(writeReqParameter->connHandle);
            
            break;
        
        default:
            break;
    }
}

/*************************************************************************\
*function to update counter value to gatt
\**************************************************************************/
void updateCounter(uint8_t count, uint8_t flags)
{
    cy_stc_ble_gatt_handle_value_pair_t myHvp;
    
    if (count < 0) 
        count = 0;
    if (count > 100) 
        count = 100;
    
    myHvp.attrHandle = CY_BLE_DEV_INT_COUNTER_CHAR_HANDLE;
    
    myHvp.value.val = (uint8_t *) &count;
    myHvp.value.actualLen = 1;
    myHvp.value.len = 1;
    if (flags == CY_BLE_GATT_DB_PEER_INITIATED)
    {
        // if it is a remote write from central BLE change here
    }
    else
    {
        Cy_BLE_GATTS_WriteAttributeValue(&myHvp);
        Cy_BLE_GATTS_SendNotification(&cy_ble_connHandle[0], &myHvp);
    }
    
    
}
void updateLEDon()
{
    cy_stc_ble_gatt_handle_value_pair_t myHvp;
    
    myHvp.attrHandle = CY_BLE_DEV_INT_RED_LED_ON_CHAR_HANDLE;
    
    uint8_t val = ledValue;
    myHvp.value.val = (uint8_t *) &val;
    myHvp.value.actualLen = 1;
    myHvp.value.len = 1;
    
    Cy_BLE_GATTS_WriteAttributeValue(&myHvp);
    Cy_BLE_GATTS_SendNotification(&cy_ble_connHandle[0], &myHvp);
    
}
void toggleLedvalue(){
    if (ledValue == 0){
        ledValue = 1;
    }
    else{
        ledValue = 0;
    }
    Cy_GPIO_Inv(LED_RED_PORT, LED_RED_NUM);
    updateLEDon();
}

void messageReceived() {
	cy_rslt_t status;
	uint32_t past_drops;


	/*
	IPC_INTR_STRUCT10->INTR = 1 << 26; // 26 = 10 (NOTIFY10) + 16 (notifies starts at bit 16)
	command = IPC_STRUCT10->DATA0;
	IPC_STRUCT10->RELEASE = 0;
	*/
    
    
	toggleLedvalue();
    

    //read message from CM4
    status = Cy_IPC_Drv_ReadMsgWord(IPC_STRUCT10, &past_drops);
    CY_ASSERT(status == CY_RSLT_SUCCESS);

    Cy_IPC_Drv_ReleaseNotify(IPC_STRUCT10, 0);
    
    // changing counter value that will be sent through BLE
    // should be changed 
    oldCounter = counter;
    counter = (uint8_t) past_drops*freq*10;
    
    if (oldCounter != counter && BleConnected == 1){
        
    updateCounter(counter, CY_BLE_GATT_DB_LOCALLY_INITIATED);
    
	}
    Cy_IPC_Drv_ClearInterrupt(IPC_INTR_STRUCT10, 0, 1 << 10);
}

/*****************************************************************************\
 * Function:    bleInterruptNotify
 * Input:       void (it is called inside of the ISR)
 * Returns:     void
 * Description: 
 *   This is called back in the BLE ISR when an event has occured and needs to
 *   be processed.
\*****************************************************************************/
void bleInterruptNotify()
{
    Cy_BLE_ProcessEvents();
    
}

int main(void)
{
    
    
    
    __enable_irq(); /* Enable global interrupts. */
    
    Cy_BLE_Start(0);
    /* Enable CM4.  CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory layout is changed. */
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR); 
    

    Cy_BLE_Start(genericEventHandler);
    
    Cy_BLE_RegisterAppHostCallback(bleInterruptNotify);
    
    // enable interrupt to receive data from CM4
    #if 1
        /* Assign the isrPC0 interrupt vector to the CM0_PC0_HANDLER register. */
    CPUSS->CM0_PC0_HANDLER = (uint32_t)&messageReceived;
    
    IPC_INTR_STRUCT_Type * myIpcIntr = Cy_IPC_Drv_GetIntrBaseAddr(MY_IPC_INTR_INDEX);
    //interrupt for IPC messages from CM4 on 10th IPC block
    uint32_t intr = Cy_IPC_Drv_GetInterruptMask(myIpcIntr);
    uint32_t releaseIntr = _FLD2VAL(IPC_INTR_STRUCT_INTR_MASK_RELEASE, intr) | MY_IPC_RELEASE_INTR_MASK;
    uint32_t acqiureIntr = _FLD2VAL(IPC_INTR_STRUCT_INTR_MASK_NOTIFY, intr) | MY_IPC_ACQUIRE_INTR_MASK;
    
    Cy_IPC_Drv_SetInterruptMask(myIpcIntr, releaseIntr, acqiureIntr);
    
     cy_stc_sysint_t intcfg = {
        #if (CY_CPU_CORTEX_M0P)
            .intrSrc = NvicMux5_IRQn,
            .cm0pSrc = cpuss_interrupts_ipc_10_IRQn,
        #else
            .intrSrc = cpuss_interrupts_ipc_10_IRQn,
        #endif /* (CY_CPU_CORTEX_M0P) */
        .intrPriority = 2u
    };
    Cy_SysInt_Init(&intcfg, messageReceived);
    NVIC_EnableIRQ(intcfg.intrSrc);
    #endif
    

   
    volatile uint32_t buttonValue = 0;
    
    
    for(;;)
    {
        if(BleConnected == 0){
            Cy_BLE_ProcessEvents();
        }
        #if 0
        /* Place your application code here. */
        #define buttonport P0_4_PORT
        #define buttonnum P0_4_NUM
        buttonValue = Cy_GPIO_Read(buttonport,buttonnum);
        //add 1 to counter (drops per 10 s) when button is pressed
        if(buttonValue == 1u && buttonReset == 0)
        {
            buttonReset = 1;
            toggleLedvalue();
            counter++;
            updateCounter(counter, CY_BLE_GATT_DB_LOCALLY_INITIATED);
        }
        else if(buttonValue == 0u)
        {
            buttonReset = 0;
        }
        #endif
        
        
        //Cy_BLE_ProcessEvents();
        //Cyble_EnterLPM(CYBLE_BLESS_SLEEP);
    }
}

/* Code references:
 *      BLE communication: https://www.youtube.com/watch?v=Aeip0hkc4YE
*/

/* [] END OF FILE */
