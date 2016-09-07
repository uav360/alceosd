/*
    AlceOSD - Graphical OSD
    Copyright (C) 2016  Luis Alves

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "alce-osd.h"


static mavlink_heartbeat_t heartbeat;
static mavlink_sys_status_t sys_status;
static mavlink_vfr_hud_t vrf_hud;
static mavlink_attitude_t attitude;

struct mavdata_state m[MAVDATA_TOTAL] = {
    [MAVDATA_HEARTBEAT] = { .data = &heartbeat },
    [MAVDATA_SYS_STATUS] = { .data = &sys_status },
    [MAVDATA_VRF_HUD] = { .data = &vrf_hud },
    [MAVDATA_ATTITUDE] = { .data = &attitude },
};

static const struct mavdata_decode_ops decode_ops[255] = {
    [MAVLINK_MSG_ID_HEARTBEAT] = { .decode = mavlink_msg_heartbeat_decode, .offset = MAVDATA_HEARTBEAT },
    [MAVLINK_MSG_ID_SYS_STATUS] = { .decode = mavlink_msg_sys_status_decode, .offset = MAVDATA_SYS_STATUS },
    [MAVLINK_MSG_ID_VFR_HUD] = { .decode = mavlink_msg_vfr_hud_decode, .offset = MAVDATA_VRF_HUD },
    [MAVLINK_MSG_ID_ATTITUDE] = { .decode = mavlink_msg_attitude_decode, .offset = MAVDATA_ATTITUDE },
    
};

void mavdata_store(mavlink_message_t *msg)
{
    const struct mavdata_decode_ops *ops = &decode_ops[msg->msgid];
    struct mavdata_state *s;
    unsigned long t;
    
    if (ops->decode != NULL) {
        s = &m[ops->offset];
        ops->decode(msg, s->data);
        t = get_millis();
        s->period = t - s->time;
        s->time = t;
    }
}

void* mavdata_get(unsigned int id)
{
    return m[id].data;
}

unsigned long mavdata_time(unsigned int id)
{
    return m[id].time;
}

unsigned long mavdata_period(unsigned int id)
{
    return m[id].period;
}
