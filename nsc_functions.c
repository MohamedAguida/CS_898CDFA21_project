/*
 * FreeRTOS Pre-Release V1.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/* CMSE includes. */
#include <arm_cmse.h>

/* NSC functions includes. */
#include "nsc_functions.h"

/* Board specific includes. */
#include "board.h"
#include "secure_port_macros.h"
#include "stdint.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"

/**
 * @brief Counter returned from NSCFunction.
 */

static uint32_t ulSecureCounter = 0;

/**
 * @brief LED port and pins.
 */
#define LED_PORT      BOARD_LED_BLUE_GPIO_PORT
#define GREEN_LED_PIN BOARD_LED_GREEN_GPIO_PIN
#define BLUE_LED_PIN  BOARD_LED_BLUE_GPIO_PIN
#define MAX_STRING_LENGTH 0x400
/**
 * @brief typedef for non-secure callback.
 */
#if defined(__IAR_SYSTEMS_ICC__)
typedef __cmse_nonsecure_call void (*NonSecureCallback_t)(void);
#else
typedef void (*NonSecureCallback_t)(void) __attribute__((cmse_nonsecure_call));
#endif
/*-----------------------------------------------------------*/

#if defined(__IAR_SYSTEMS_ICC__)
__cmse_nonsecure_entry
#else
__attribute__((cmse_nonsecure_entry))
#endif
    uint32_t
    NSCFunction(Callback_t pxCallback)
{
    NonSecureCallback_t pxNonSecureCallback;

    /* Return function pointer with cleared LSB. */
    pxNonSecureCallback = (NonSecureCallback_t)cmse_nsfptr_create(pxCallback);

    /* Invoke the supplied callback. */
    pxNonSecureCallback();

    /* Increment the secure side counter. */
    ulSecureCounter += 1;

    /* Return the secure side counter. */
    return ulSecureCounter;
}
/*-----------------------------------------------------------*/

#if defined(__IAR_SYSTEMS_ICC__)
__cmse_nonsecure_entry
#else
__attribute__((cmse_nonsecure_entry))
#endif
    void
    vToggleGreenLED(void)
{
    /* Toggle the on-board green LED. */
    for (int i=0; i<10000000; i++){}
    //GPIO_PortToggle(GPIO, LED_PORT, (1U << GREEN_LED_PIN));
}
/*-----------------------------------------------------------*/

#if defined(__IAR_SYSTEMS_ICC__)
__cmse_nonsecure_entry
#else
__attribute__((cmse_nonsecure_entry))
#endif
    void
    vToggleBlueLED()
{
    /* Toggle the on-board blue LED. */
    GPIO_PortToggle(GPIO, LED_PORT, (1U << BLUE_LED_PIN));
}
/*-----------------------------------------------------------*/

#if defined(__IAR_SYSTEMS_ICC__)
__cmse_nonsecure_entry
#else
__attribute__((cmse_nonsecure_entry))
#endif
    uint32_t
    getSystemCoreClock(void)
{
    /* Return the secure side SystemCoreClock. */
    return SystemCoreClock;
}


/* strnlen function implementation for arm compiler */
#if defined(__arm__)
size_t strnlen(const char *s, size_t maxLength)
{
    size_t length = 0;
    while ((length <= maxLength) && (*s))
    {
        s++;
        length++;
    }
    return length;
}
#endif

#if defined(__IAR_SYSTEMS_ICC__)
__cmse_nonsecure_entry
#else
__attribute__((cmse_nonsecure_entry))
#endif
     void DbgConsole_Printf_NSE(char const *s)
{
    size_t string_length;
    /* Access to non-secure memory from secure world has to be properly validated */
    /* Check whether string is properly terminated */
    string_length = strnlen(s, MAX_STRING_LENGTH);
    if ((string_length == MAX_STRING_LENGTH) && (s[string_length] != '\0'))
    {
        PRINTF("String too long or invalid string termination!\r\n");
        while (1)
            ;
    }

    /* Check whether string is located in non-secure memory */
    /* Due to the bug in GCC 10 cmse_check_address_range() always fail, do not call it, see GCC Bugzilla - Bug 99157 */
#if (__GNUC__ != 10)
    if (cmse_check_address_range((void *)s, string_length, CMSE_NONSECURE | CMSE_MPU_READ) == NULL)
    {
        PRINTF("String is not located in normal world!\r\n");
        while (1)
            ;
    }
#endif
    PRINTF(s);
}

#if defined(__IAR_SYSTEMS_ICC__)
__cmse_nonsecure_entry
#else
__attribute__((cmse_nonsecure_entry))
#endif
uint32_t
	vVictim_func(uint32_t c)
{
	for (int i=0; i<10000000; i++){}
	//vPrintString("this is value :  is from secure world\r\n");

    return c;
}
/*-----------------------------------------------------------*/
