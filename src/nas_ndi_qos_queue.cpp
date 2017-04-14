/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/*
 * filename: nas_ndi_qos_queue.cpp
 */

#include "std_error_codes.h"
#include "nas_ndi_event_logs.h"
#include "nas_ndi_int.h"
#include "nas_ndi_utils.h"
#include "nas_ndi_qos_utl.h"
#include "sai.h"
#include "dell-base-qos.h" //from yang model
#include "nas_ndi_qos.h"
#include "nas_ndi_switch.h"

#include <stdio.h>
#include <vector>
#include <unordered_map>

#ifdef USING_BROADCOM_SAI
/*
 * Extreme change - redefine names so that they match the names
 * defined by the Broadcom SAI.
 */
#define sai_queue_stat_t sai_queue_stat_counter_t
#endif

/**
 * This function set queue attribute
 * @param ndi_port_id
 * @param ndi_queue_id
 * @param wred_id
 * @return standard error
 */
t_std_error ndi_qos_set_queue_wred_id(ndi_port_t ndi_port_id,
                                    ndi_obj_id_t ndi_queue_id,
                                    ndi_obj_id_t wred_id)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, 0);
    }

    sai_attribute_t attr = {0};
    attr.id = SAI_QUEUE_ATTR_WRED_PROFILE_ID;
    attr.value.oid = ndi2sai_wred_profile_id(wred_id);

    if ((sai_ret = ndi_sai_qos_queue_api(ndi_db_ptr)->
                        set_queue_attribute(ndi2sai_queue_id(ndi_queue_id), &attr))
                         != SAI_STATUS_SUCCESS) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                "queue set fails: npu_id %u\n",
                ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, sai_ret);
    }

    return STD_ERR_OK;
}

/**
 * This function set queue attribute
 * @param ndi_port_id
 * @param ndi_queue_id
 * @param buffer_profile_id
 * @return standard error
 */
t_std_error ndi_qos_set_queue_buffer_profile_id(ndi_port_t ndi_port_id,
                                    ndi_obj_id_t ndi_queue_id,
                                    ndi_obj_id_t buffer_profile_id)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, 0);
    }

    sai_attribute_t attr = {0};
    attr.id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID;
    attr.value.oid = ndi2sai_buffer_profile_id(buffer_profile_id);

    if ((sai_ret = ndi_sai_qos_queue_api(ndi_db_ptr)->
                        set_queue_attribute(ndi2sai_queue_id(ndi_queue_id), &attr))
                         != SAI_STATUS_SUCCESS) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                "queue set fails: npu_id %u\n",
                ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, sai_ret);
    }

    return STD_ERR_OK;
}

/**
 * This function set queue attribute
 * @param ndi_port_id
 * @param ndi_queue_id
 * @param scheudler_profile_id
 * @return standard error
 */
t_std_error ndi_qos_set_queue_scheduler_profile_id(ndi_port_t ndi_port_id,
                                    ndi_obj_id_t ndi_queue_id,
                                    ndi_obj_id_t scheduler_profile_id)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, 0);
    }

    sai_attribute_t attr = {0};
    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr.value.oid = ndi2sai_scheduler_profile_id(scheduler_profile_id);

    if ((sai_ret = ndi_sai_qos_queue_api(ndi_db_ptr)->
                        set_queue_attribute(ndi2sai_queue_id(ndi_queue_id), &attr))
                         != SAI_STATUS_SUCCESS) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                "queue set fails: npu_id %u\n",
                ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, sai_ret);
    }

    return STD_ERR_OK;
}

/**
 * This function gets all attributes of a queue
 * @param ndi_port_id
 * @param ndi_queue_id
 * @param[out] info queue attributes info
 * return standard error
 */
t_std_error ndi_qos_get_queue_attribute(ndi_port_t ndi_port_id,
        ndi_obj_id_t ndi_queue_id,
        ndi_qos_queue_attribute_t * info)
{
    //set all the flags and call sai
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, 0);
    }

    std::vector<sai_attribute_t> attr_list;
    sai_attribute_t attr={0};
    attr.id = SAI_QUEUE_ATTR_TYPE;
    attr_list.push_back(attr);
    attr.id = SAI_QUEUE_ATTR_WRED_PROFILE_ID;
    attr_list.push_back(attr);
    attr.id = SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID;
    attr_list.push_back(attr);

    if ((sai_ret = ndi_sai_qos_queue_api(ndi_db_ptr)->
                        get_queue_attribute(ndi2sai_queue_id(ndi_queue_id),
                                attr_list.size(),
                                &attr_list[0]))
                         != SAI_STATUS_SUCCESS) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                "queue get fails: npu_id %u, ndi_queue_id 0x%016lx\n",
                ndi_port_id.npu_id, ndi_queue_id);
        return STD_ERR(QOS, CFG, sai_ret);
    }

    for (auto attr: attr_list) {
        if (attr.id == SAI_QUEUE_ATTR_TYPE) {
            info->type = (attr.value.u32 == SAI_QUEUE_TYPE_UNICAST?
                            BASE_QOS_QUEUE_TYPE_UCAST:
                            (attr.value.u32 == SAI_QUEUE_TYPE_MULTICAST?
                             BASE_QOS_QUEUE_TYPE_MULTICAST: BASE_QOS_QUEUE_TYPE_NONE));
        }

        if (attr.id == SAI_QUEUE_ATTR_WRED_PROFILE_ID)
            info->wred_id = sai2ndi_wred_profile_id(attr.value.oid);

        if (attr.id == SAI_QUEUE_ATTR_BUFFER_PROFILE_ID)
            info->buffer_profile = sai2ndi_buffer_profile_id(attr.value.oid);

        if (attr.id == SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID)
            info->scheduler_profile = sai2ndi_scheduler_profile_id(attr.value.oid);
    }

    return STD_ERR_OK;
}

/**
 * This function gets the total number of queues on a port
 * @param ndi_port_id
 * @Return standard error code
 */
uint_t ndi_qos_get_number_of_queues(ndi_port_t ndi_port_id)
{
    static uint_t fp_queue_count = 0;
    static uint_t cpu_queue_count = 0;

    if (fp_queue_count == 0 || cpu_queue_count == 0) {
        ndi_switch_get_queue_numbers(ndi_port_id.npu_id, NULL, NULL, &fp_queue_count, &cpu_queue_count);
    }

    if (ndi_port_id.npu_port == 0)
        return cpu_queue_count;
    else
        return fp_queue_count;

}

/**
 * This function gets the list of queues of a port
 * @param ndi_port_id
 * @param count size of the queue_list
 * @param[out] ndi_queue_id_list[] to be filled with either the number of queues
 *            that the port owns or the size of array itself, whichever is less.
 * @Return Number of queues that the port owns.
 */
uint_t ndi_qos_get_queue_id_list(ndi_port_t ndi_port_id,
                                uint_t count,
                                ndi_obj_id_t *ndi_queue_id_list)
{
    /* get queue list */
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);

        return 0;
    }

    sai_attribute_t sai_attr;
    std::vector<sai_object_id_t> sai_queue_id_list(count);

    sai_attr.id = SAI_PORT_ATTR_QOS_QUEUE_LIST;
    sai_attr.value.objlist.count = count;
    sai_attr.value.objlist.list = &sai_queue_id_list[0];

    sai_object_id_t sai_port;
    if (ndi_sai_port_id_get(ndi_port_id.npu_id, ndi_port_id.npu_port, &sai_port) != STD_ERR_OK) {
        return 0;
    }

    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    sai_ret = ndi_sai_qos_port_api(ndi_db_ptr)->
                        get_port_attribute(sai_port,
                                    1, &sai_attr);

    if (sai_ret != SAI_STATUS_SUCCESS  &&
        sai_ret != SAI_STATUS_BUFFER_OVERFLOW) {
        return 0;
    }

    // copy out sai-returned queue ids to nas
    if (ndi_queue_id_list) {
        for (uint_t i = 0; (i< sai_attr.value.objlist.count) && (i < count); i++) {
            ndi_queue_id_list[i] = sai2ndi_queue_id(sai_attr.value.objlist.list[i]);
        }
    }

    return sai_attr.value.objlist.count;

}


static const std::unordered_map<BASE_QOS_QUEUE_STAT_t, sai_queue_stat_t, std::hash<int>>
    nas2sai_queue_counter_type = {
        {BASE_QOS_QUEUE_STAT_PACKETS, SAI_QUEUE_STAT_PACKETS},
        {BASE_QOS_QUEUE_STAT_BYTES, SAI_QUEUE_STAT_BYTES},
        {BASE_QOS_QUEUE_STAT_DROPPED_PACKETS, SAI_QUEUE_STAT_DROPPED_PACKETS},
        {BASE_QOS_QUEUE_STAT_DROPPED_BYTES, SAI_QUEUE_STAT_DROPPED_BYTES},
        {BASE_QOS_QUEUE_STAT_GREEN_PACKETS, SAI_QUEUE_STAT_GREEN_PACKETS},
        {BASE_QOS_QUEUE_STAT_GREEN_BYTES, SAI_QUEUE_STAT_GREEN_BYTES},
        {BASE_QOS_QUEUE_STAT_GREEN_DROPPED_PACKETS, SAI_QUEUE_STAT_GREEN_DROPPED_PACKETS},
        {BASE_QOS_QUEUE_STAT_GREEN_DROPPED_BYTES, SAI_QUEUE_STAT_GREEN_DROPPED_BYTES},
        {BASE_QOS_QUEUE_STAT_YELLOW_PACKETS, SAI_QUEUE_STAT_YELLOW_PACKETS},
        {BASE_QOS_QUEUE_STAT_YELLOW_BYTES, SAI_QUEUE_STAT_YELLOW_BYTES},
        {BASE_QOS_QUEUE_STAT_YELLOW_DROPPED_PACKETS, SAI_QUEUE_STAT_YELLOW_DROPPED_PACKETS},
        {BASE_QOS_QUEUE_STAT_YELLOW_DROPPED_BYTES, SAI_QUEUE_STAT_YELLOW_DROPPED_BYTES},
        {BASE_QOS_QUEUE_STAT_RED_PACKETS, SAI_QUEUE_STAT_RED_PACKETS},
        {BASE_QOS_QUEUE_STAT_RED_BYTES, SAI_QUEUE_STAT_RED_BYTES},
        {BASE_QOS_QUEUE_STAT_RED_DROPPED_PACKETS, SAI_QUEUE_STAT_RED_DROPPED_PACKETS},
        {BASE_QOS_QUEUE_STAT_RED_DROPPED_BYTES, SAI_QUEUE_STAT_RED_DROPPED_BYTES},
        {BASE_QOS_QUEUE_STAT_GREEN_DISCARD_DROPPED_PACKETS, SAI_QUEUE_STAT_GREEN_DISCARD_DROPPED_PACKETS},
        {BASE_QOS_QUEUE_STAT_GREEN_DISCARD_DROPPED_BYTES, SAI_QUEUE_STAT_GREEN_DISCARD_DROPPED_BYTES},
        {BASE_QOS_QUEUE_STAT_YELLOW_DISCARD_DROPPED_PACKETS, SAI_QUEUE_STAT_YELLOW_DISCARD_DROPPED_PACKETS},
        {BASE_QOS_QUEUE_STAT_YELLOW_DISCARD_DROPPED_BYTES, SAI_QUEUE_STAT_YELLOW_DISCARD_DROPPED_BYTES},
        {BASE_QOS_QUEUE_STAT_RED_DISCARD_DROPPED_PACKETS, SAI_QUEUE_STAT_RED_DISCARD_DROPPED_PACKETS},
        {BASE_QOS_QUEUE_STAT_RED_DISCARD_DROPPED_BYTES, SAI_QUEUE_STAT_RED_DISCARD_DROPPED_BYTES},
        {BASE_QOS_QUEUE_STAT_DISCARD_DROPPED_PACKETS, SAI_QUEUE_STAT_DISCARD_DROPPED_PACKETS},
        {BASE_QOS_QUEUE_STAT_DISCARD_DROPPED_BYTES, SAI_QUEUE_STAT_DISCARD_DROPPED_BYTES},
#ifndef USING_BROADCOM_SAI
        {BASE_QOS_QUEUE_STAT_CURRENT_OCCUPANCY_BYTES, SAI_QUEUE_STAT_CURR_OCCUPANCY_BYTES},
        {BASE_QOS_QUEUE_STAT_WATERMARK_BYTES, SAI_QUEUE_STAT_WATERMARK_BYTES},
        {BASE_QOS_QUEUE_STAT_SHARED_CURRENT_OCCUPANCY_BYTES, SAI_QUEUE_STAT_SHARED_CURR_OCCUPANCY_BYTES},
        {BASE_QOS_QUEUE_STAT_SHARED_WATERMARK_BYTES, SAI_QUEUE_STAT_SHARED_WATERMARK_BYTES},
#endif
    };

static void _fill_counter_stat_by_type(sai_queue_stat_t type, uint64_t val,
        nas_qos_queue_stat_counter_t *stat )
{
    switch(type) {
    case SAI_QUEUE_STAT_PACKETS:
        stat->packets = val;
        break;
    case SAI_QUEUE_STAT_BYTES:
        stat->bytes = val;
        break;
    case SAI_QUEUE_STAT_DROPPED_PACKETS:
        stat->dropped_packets = val;
        break;
    case SAI_QUEUE_STAT_DROPPED_BYTES:
        stat->dropped_bytes = val;
        break;
    case SAI_QUEUE_STAT_GREEN_PACKETS:
        stat->green_packets = val;
        break;
    case SAI_QUEUE_STAT_GREEN_BYTES:
        stat->green_bytes = val;
        break;
    case SAI_QUEUE_STAT_GREEN_DROPPED_PACKETS:
        stat->green_dropped_packets = val;
        break;
    case SAI_QUEUE_STAT_GREEN_DROPPED_BYTES:
        stat->green_dropped_bytes = val;
        break;
    case SAI_QUEUE_STAT_YELLOW_PACKETS:
        stat->yellow_packets = val;
        break;
    case SAI_QUEUE_STAT_YELLOW_BYTES:
        stat->yellow_bytes = val;
        break;
    case SAI_QUEUE_STAT_YELLOW_DROPPED_PACKETS:
        stat->yellow_dropped_packets = val;
        break;
    case SAI_QUEUE_STAT_YELLOW_DROPPED_BYTES:
        stat->yellow_dropped_bytes = val;
        break;
    case SAI_QUEUE_STAT_RED_PACKETS:
        stat->red_packets = val;
        break;
    case SAI_QUEUE_STAT_RED_BYTES:
        stat->red_bytes = val;
        break;
    case SAI_QUEUE_STAT_RED_DROPPED_PACKETS:
        stat->red_dropped_packets = val;
        break;
    case SAI_QUEUE_STAT_RED_DROPPED_BYTES:
        stat->red_dropped_bytes = val;
        break;
    case SAI_QUEUE_STAT_GREEN_DISCARD_DROPPED_PACKETS:
        stat->green_discard_dropped_packets = val;
        break;
    case SAI_QUEUE_STAT_GREEN_DISCARD_DROPPED_BYTES:
        stat->green_discard_dropped_bytes = val;
        break;
    case SAI_QUEUE_STAT_YELLOW_DISCARD_DROPPED_PACKETS:
        stat->yellow_discard_dropped_packets = val;
        break;
    case SAI_QUEUE_STAT_YELLOW_DISCARD_DROPPED_BYTES:
        stat->yellow_discard_dropped_bytes = val;
        break;
    case SAI_QUEUE_STAT_RED_DISCARD_DROPPED_PACKETS:
        stat->red_discard_dropped_packets = val;
        break;
    case SAI_QUEUE_STAT_RED_DISCARD_DROPPED_BYTES:
        stat->red_discard_dropped_bytes = val;
        break;
    case SAI_QUEUE_STAT_DISCARD_DROPPED_PACKETS:
        stat->discard_dropped_packets = val;
        break;
    case SAI_QUEUE_STAT_DISCARD_DROPPED_BYTES:
        stat->discard_dropped_bytes = val;
        break;
#ifndef USING_BROADCOM_SAI
    case SAI_QUEUE_STAT_CURR_OCCUPANCY_BYTES:
        stat->current_occupancy_bytes = val;
        break;
    case SAI_QUEUE_STAT_WATERMARK_BYTES:
        stat->watermark_bytes = val;
        break;
    case SAI_QUEUE_STAT_SHARED_CURR_OCCUPANCY_BYTES:
        stat->shared_current_occupancy_bytes = val;
        break;
    case SAI_QUEUE_STAT_SHARED_WATERMARK_BYTES:
        stat->shared_watermark_bytes = val;
        break;
#endif
    default:
        break;
    }
}


/**
 * This function gets the queue statistics
 * @param ndi_port_id
 * @param ndi_queue_id
 * @param list of queue counter types to query
 * @param number of queue counter types specified
 * @param[out] counter stats
  * return standard error
 */
t_std_error ndi_qos_get_queue_stats(ndi_port_t ndi_port_id,
                                ndi_obj_id_t ndi_queue_id,
                                BASE_QOS_QUEUE_STAT_t *counter_ids,
                                uint_t number_of_counters,
                                nas_qos_queue_stat_counter_t *stats)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, 0);
    }

    std::vector<sai_queue_stat_t> counter_id_list;
    std::vector<uint64_t> counters(number_of_counters);

    for (uint_t i= 0; i<number_of_counters; i++) {
        counter_id_list.push_back(nas2sai_queue_counter_type.at(counter_ids[i]));
    }
    if ((sai_ret = ndi_sai_qos_queue_api(ndi_db_ptr)->
                        get_queue_stats(ndi2sai_queue_id(ndi_queue_id),
                                &counter_id_list[0],
                                number_of_counters,
                                &counters[0]))
                         != SAI_STATUS_SUCCESS) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                "queue get stats fails: npu_id %u\n",
                ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, sai_ret);
    }

    // copy the stats out
    for (uint i= 0; i<number_of_counters; i++) {
        _fill_counter_stat_by_type(counter_id_list[i], counters[i], stats);
    }

    return STD_ERR_OK;
}

/**
 * This function clears the queue statistics
 * @param ndi_port_id
 * @param ndi_queue_id
 * @param list of queue counter types to clear
 * @param number of queue counter types specified
 * return standard error
 */
t_std_error ndi_qos_clear_queue_stats(ndi_port_t ndi_port_id,
                                ndi_obj_id_t ndi_queue_id,
                                BASE_QOS_QUEUE_STAT_t *counter_ids,
                                uint_t number_of_counters)
{
    sai_status_t sai_ret = SAI_STATUS_FAILURE;
    nas_ndi_db_t *ndi_db_ptr = ndi_db_ptr_get(ndi_port_id.npu_id);
    if (ndi_db_ptr == NULL) {
        EV_LOGGING(NDI, DEBUG, "NDI-QOS",
                      "npu_id %d not exist\n", ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, 0);
    }

    std::vector<sai_queue_stat_t> counter_id_list;
    std::vector<uint64_t> counters(number_of_counters);

    for (uint_t i= 0; i<number_of_counters; i++) {
        counter_id_list.push_back(nas2sai_queue_counter_type.at(counter_ids[i]));
    }
    if ((sai_ret = ndi_sai_qos_queue_api(ndi_db_ptr)->
                        clear_queue_stats(ndi2sai_queue_id(ndi_queue_id),
                                &counter_id_list[0],
                                number_of_counters))
                         != SAI_STATUS_SUCCESS) {
        EV_LOGGING(NDI, NOTICE, "NDI-QOS",
                "queue clear stats fails: npu_id %u\n",
                ndi_port_id.npu_id);
        return STD_ERR(QOS, CFG, sai_ret);
    }

    return STD_ERR_OK;

}

