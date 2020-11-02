/***************************************************************************//**
* \file CY_BLE_hts_config.c
* \version 2.0
* 
* \brief
*  This file contains the source code of initialization of the config structure
*  for the Health Thermometer Service.
*
********************************************************************************
* \copyright
* Copyright 2017-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#include "ble/cy_ble_hts.h"

#if(CY_BLE_MODE_PROFILE && defined(CY_BLE_HTS))

#ifdef CY_BLE_HTS_SERVER
static const cy_stc_ble_htss_t cy_ble_htss =
{
    0x000Cu,    /* Handle of the HTS service */
    {
        
        /* Temperature Measurement characteristic */
        {
            0x000Eu, /* Handle of the Temperature Measurement characteristic */ 
            
            /* Array of Descriptors handles */
            {
                0x000Fu, /* Handle of the Client Characteristic Configuration descriptor */ 
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE, 
            }, 
        }, 
        
        /* Temperature Type characteristic */
        {
            0x0011u, /* Handle of the Temperature Type characteristic */ 
            
            /* Array of Descriptors handles */
            {
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE, 
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE, 
            }, 
        }, 
        
        /* Intermediate Temperature characteristic */
        {
            0x0013u, /* Handle of the Intermediate Temperature characteristic */ 
            
            /* Array of Descriptors handles */
            {
                0x0014u, /* Handle of the Client Characteristic Configuration descriptor */ 
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE, 
            }, 
        }, 
        
        /* Measurement Interval characteristic */
        {
            0x0016u, /* Handle of the Measurement Interval characteristic */ 
            
            /* Array of Descriptors handles */
            {
                0x0017u, /* Handle of the Client Characteristic Configuration descriptor */ 
                0x0018u, /* Handle of the Valid Range descriptor */ 
            }, 
        }, 
    },
};
#endif /* CY_BLE_HTS_SERVER */

/**
* \addtogroup group_globals
* @{
*/

/** The configuration structure for the Health Thermometer Service. */
cy_stc_ble_hts_config_t cy_ble_htsConfig =
{
    /* Service GATT DB handles structure */
    #ifdef CY_BLE_HTS_SERVER
    .htss = &cy_ble_htss,
    #else
    .htss = NULL,
    #endif /* CY_BLE_HTS_SERVER */

    /* An application layer event callback function to receive service events from the BLE Component. */
    .callbackFunc = NULL,
};

/** @} group_globals */

#endif /* (CY_BLE_MODE_PROFILE && defined(CY_BLE_HTS)) */

/* [] END OF FILE */
