/*
 * hid_cdc_msc_class.c
 *
 *  Created on: Feb 17, 2019
 *      Author: khockuba
 */

#include "usbd_desc.h"
#include "usbd_ctlreq.h"

#include "usbd_msc.h"
#include "usbd_cdc.h"
#include "usbd_hid.h"

#define HID_INTERFACE 0x00
#define CDC_INTERFACE 0x01
#define MSC_INTERFACE 0x02

uint8_t  USBD_Wrapper_Init          (USBD_HandleTypeDef *pdev , uint8_t cfgidx);
uint8_t  USBD_Wrapper_DeInit        (USBD_HandleTypeDef *pdev , uint8_t cfgidx);
uint8_t  USBD_Setup                 (USBD_HandleTypeDef *pdev , USBD_SetupReqTypedef  *req);

uint8_t  USBD_EP0TxSent             (USBD_HandleTypeDef *pdev );
uint8_t  USBD_EP0RxReady            (USBD_HandleTypeDef *pdev );

uint8_t  USBD_DataIn                (USBD_HandleTypeDef *pdev , uint8_t epnum);
uint8_t  USBD_DataOut               (USBD_HandleTypeDef *pdev , uint8_t epnum);
uint8_t  USBD_WrapperSOF            (USBD_HandleTypeDef *pdev);
uint8_t  USBD_IsoINIncomplete       (USBD_HandleTypeDef *pdev , uint8_t epnum);
uint8_t  USBD_IsoOUTIncomplete      (USBD_HandleTypeDef *pdev , uint8_t epnum);

uint8_t  *USBD_GetHSConfigDescriptor (uint16_t *length);
uint8_t  *USBD_GetFSConfigDescriptor (uint16_t *length);
uint8_t  *USBD_GetOtherSpeedConfigDescriptor(uint16_t *length);
uint8_t  *USBD_GetDeviceQualifierDescriptor(uint16_t *length);

USBD_ClassTypeDef  USBD_WRAPPER =
{
  USBD_Wrapper_Init,
  USBD_Wrapper_DeInit,
  USBD_Setup,
  USBD_EP0TxSent, /*EP0_TxSent*/
  USBD_EP0RxReady, /*EP0_RxReady*/
  USBD_DataIn, /*DataIn*/
  USBD_DataOut, /*DataOut*/
  USBD_WrapperSOF, /*SOF */
  USBD_IsoINIncomplete,
  USBD_IsoOUTIncomplete,
  USBD_GetHSConfigDescriptor,
  USBD_GetFSConfigDescriptor,
  USBD_GetOtherSpeedConfigDescriptor,
  USBD_GetDeviceQualifierDescriptor,
};

uint8_t USBD_Wrapper_Init(USBD_HandleTypeDef *pdev , uint8_t cfgidx) {
    USBD_MSC.Init(pdev, cfgidx);
    USBD_CDC.Init(pdev, cfgidx);
    USBD_HID.Init(pdev, cfgidx);
    return USBD_OK;
}

uint8_t USBD_Wrapper_DeInit(USBD_HandleTypeDef *pdev , uint8_t cfgidx) {
    USBD_MSC.DeInit(pdev, cfgidx);
    USBD_CDC.DeInit(pdev, cfgidx);
    USBD_HID.DeInit(pdev, cfgidx);
    return USBD_OK;
}

uint8_t USBD_Setup(USBD_HandleTypeDef *pdev , USBD_SetupReqTypedef  *req) {
    switch(req->bmRequest & USB_REQ_RECIPIENT_MASK) {
        case USB_REQ_RECIPIENT_INTERFACE:
            if (req->wIndex == HID_INTERFACE) {
                return USBD_HID.Setup(pdev, req);
            }
            else if (req->wIndex == CDC_INTERFACE){
                return USBD_CDC.Setup(pdev, req);
            }
            else if (req->wIndex == MSC_INTERFACE) {
                return USBD_MSC.Setup(pdev, req);
            }
            break;
        case USB_REQ_RECIPIENT_ENDPOINT:
            if (req->wIndex == HID_EPIN_ADDR) {
                return USBD_HID.Setup(pdev, req);
            } else if (req->wIndex == CDC_IN_EP) {
                return USBD_CDC.Setup(pdev, req);
            }
            else if (req->wIndex == MSC_EPIN_ADDR) {
                return USBD_MSC.Setup(pdev, req);
            }
            break;
    }
    return USBD_OK;
}

uint8_t USBD_EP0TxSent(USBD_HandleTypeDef *pdev ) {
    uint8_t ret = USBD_OK;
    if (USBD_HID.EP0_TxSent)
        ret |= USBD_HID.EP0_TxSent(pdev);
    if (USBD_CDC.EP0_TxSent)
        ret |= USBD_CDC.EP0_TxSent(pdev);
    if (USBD_MSC.EP0_TxSent)
        ret |= USBD_MSC.EP0_TxSent(pdev);
    return ret;
}

uint8_t USBD_EP0RxReady(USBD_HandleTypeDef *pdev ) {
    uint8_t ret = USBD_OK;
    if (USBD_HID.EP0_RxReady)
        ret |= USBD_HID.EP0_RxReady(pdev);
    if (USBD_CDC.EP0_RxReady)
        ret |= USBD_CDC.EP0_RxReady(pdev);
    if (USBD_MSC.EP0_RxReady)
        ret |= USBD_MSC.EP0_RxReady(pdev);
    return ret;
}

uint8_t USBD_DataIn(USBD_HandleTypeDef *pdev , uint8_t epnum) {
    if (epnum == (MSC_EPIN_ADDR &~ 0x80)) {

    }
    else if(epnum == (HID_EPIN_ADDR &~ 0x80)) {

    }
    else if (epnum == (CDC_IN_EP &~ 0x80)) {

    }
    return USBD_OK;
}

uint8_t USBD_DataOut(USBD_HandleTypeDef *pdev , uint8_t epnum) {
    if (epnum == (MSC_EPOUT_ADDR &~ 0x80)) {

    }
    else if (epnum == (CDC_OUT_EP &~ 0x80)) {

    }
    return USBD_OK;
}

uint8_t USBD_WrapperSOF(USBD_HandleTypeDef *pdev) {
    int ret = USBD_OK;
    if (USBD_MSC.SOF)
        ret |= USBD_MSC.SOF(pdev);
    if (USBD_CDC.SOF)
        ret |= USBD_CDC.SOF(pdev);
    if (USBD_HID.SOF)
        ret |= USBD_HID.SOF(pdev);
    return ret;
}

uint8_t  USBD_IsoINIncomplete(USBD_HandleTypeDef *pdev , uint8_t epnum) {
    return USBD_OK;
}

uint8_t  USBD_IsoOUTIncomplete(USBD_HandleTypeDef *pdev , uint8_t epnum) {
    return USBD_OK;
}

uint8_t *USBD_GetHSConfigDescriptor (uint16_t *length) {
    return USBD_OK;
}

uint8_t *USBD_GetFSConfigDescriptor (uint16_t *length) {
    return USBD_OK;
}

uint8_t *USBD_GetOtherSpeedConfigDescriptor(uint16_t *length) {
    return USBD_OK;
}

uint8_t *USBD_GetDeviceQualifierDescriptor(uint16_t *length) {
    return USBD_OK;
}
