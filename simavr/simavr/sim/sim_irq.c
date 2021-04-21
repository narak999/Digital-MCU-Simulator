/*
	sim_irq.c

	Copyright 2008, 2009 Michel Pollet <buserror@gmail.com>

 	This file is part of simavr.

	simavr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	simavr is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with simavr.  If not, see <http://www.gnu.org/licenses/>.

	simavr modified by Jonathan Howard, ECEN 404 Team 10, Spring 2021
	Texas A&M University
	This modification is licensed under the same GNU GPL v3 as simavr,
	and the license has been included in the main directory of the project as
	GPL.txt.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sim_irq.h"
#include "ftd2xx.h"


extern FT_HANDLE deviceHandle;
extern FT_STATUS ftStatus;

// internal structure for a hook, never seen by the notify procs
typedef struct avr_irq_hook_t {
	struct avr_irq_hook_t * next;
	int busy;	// prevent reentrance of callbacks

	struct avr_irq_t * chain;	// raise the IRQ on this too - optional if "notify" is on
	avr_irq_notify_t notify;	// called when IRQ is raised - optional if "chain" is on
	void * param;				// "notify" parameter
} avr_irq_hook_t;

static void
_avr_irq_pool_add(
		avr_irq_pool_t * pool,
		avr_irq_t * irq)
{
	int insert = 0;
	/* lookup a slot */
	for (; insert < pool->count && pool->irq[insert]; insert++)
		;
	if (insert == pool->count) {
		if ((pool->count & 0xf) == 0) {
			pool->irq = (avr_irq_t**)realloc(pool->irq,
					(pool->count + 16) * sizeof(avr_irq_t *));
		}
		pool->count++;
	}
	pool->irq[insert] = irq;
	irq->pool = pool;
}

static void
_avr_irq_pool_remove(
		avr_irq_pool_t * pool,
		avr_irq_t * irq)
{
	for (int i = 0; i < pool->count; i++)
		if (pool->irq[i] == irq) {
			pool->irq[i] = 0;
			return;
		}
}

void
avr_init_irq(
		avr_irq_pool_t * pool,
		avr_irq_t * irq,
		uint32_t base,
		uint32_t count,
		const char ** names /* optional */)
{
	memset(irq, 0, sizeof(avr_irq_t) * count);

	for (int i = 0; i < count; i++) {
		irq[i].irq = base + i;
		irq[i].flags = IRQ_FLAG_INIT;
		if (pool)
			_avr_irq_pool_add(pool, &irq[i]);
		if (names && names[i])
			irq[i].name = strdup(names[i]);
		else {
			printf("WARNING %s() with NULL name for irq %d.\n", __func__, irq[i].irq);
		}
	}

}

avr_irq_t *
avr_alloc_irq(
		avr_irq_pool_t * pool,
		uint32_t base,
		uint32_t count,
		const char ** names /* optional */)
{
	avr_irq_t * irq = (avr_irq_t*)malloc(sizeof(avr_irq_t) * count);
	avr_init_irq(pool, irq, base, count, names);
	for (int i = 0; i < count; i++)
		irq[i].flags |= IRQ_FLAG_ALLOC;
	return irq;
}

static avr_irq_hook_t *
_avr_alloc_irq_hook(
		avr_irq_t * irq)
{
	avr_irq_hook_t *hook = malloc(sizeof(avr_irq_hook_t));
	memset(hook, 0, sizeof(avr_irq_hook_t));
	hook->next = irq->hook;
	irq->hook = hook;
	return hook;
}

void
avr_free_irq(
		avr_irq_t * irq,
		uint32_t count)
{
	if (!irq || !count)
		return;
	for (int i = 0; i < count; i++) {
		avr_irq_t * iq = irq + i;
		if (iq->pool)
			_avr_irq_pool_remove(iq->pool, iq);
		if (iq->name)
			free((char*)iq->name);
		iq->name = NULL;
		// purge hooks
		avr_irq_hook_t *hook = iq->hook;
		while (hook) {
			avr_irq_hook_t * next = hook->next;
			free(hook);
			hook = next;
		}
		iq->hook = NULL;
	}
	// if that irq list was allocated by us, free it
	if (irq->flags & IRQ_FLAG_ALLOC)
		free(irq);
}

void
avr_irq_register_notify(
		avr_irq_t * irq,
		avr_irq_notify_t notify,
		void * param)
{
	if (!irq || !notify)
		return;

	avr_irq_hook_t *hook = irq->hook;
	while (hook) {
		if (hook->notify == notify && hook->param == param)
			return;	// already there
		hook = hook->next;
	}
	hook = _avr_alloc_irq_hook(irq);
	hook->notify = notify;
	hook->param = param;
}

void
avr_irq_unregister_notify(
		avr_irq_t * irq,
		avr_irq_notify_t notify,
		void * param)
{
	avr_irq_hook_t *hook, *prev;
	if (!irq || !notify)
		return;

	hook = irq->hook;
	prev = NULL;
	while (hook) {
		if (hook->notify == notify && hook->param == param) {
			if ( prev )
				prev->next = hook->next;
			else
				irq->hook = hook->next;
			free(hook);
			return;
		}
		prev = hook;
		hook = hook->next;
	}
}

void
avr_raise_irq_float(
		avr_irq_t * irq,
		uint32_t value,
		int floating)
{
	if (!irq)
		return ;
	uint32_t output = (irq->flags & IRQ_FLAG_NOT) ? !value : value;
	// if value is the same but it's the first time, raise it anyway
	if (irq->value == output &&
			(irq->flags & IRQ_FLAG_FILTERED) && !(irq->flags & IRQ_FLAG_INIT))
		return;
	irq->flags &= ~(IRQ_FLAG_INIT | IRQ_FLAG_FLOATING);
	if (floating)
		irq->flags |= IRQ_FLAG_FLOATING;
	avr_irq_hook_t *hook = irq->hook;
	while (hook) {
		avr_irq_hook_t * next = hook->next;
			// prevents reentrance / endless calling loops
		if (hook->busy == 0) {
			hook->busy++;
			if (hook->notify)
				hook->notify(irq, output,  hook->param);
			if (hook->chain)
				avr_raise_irq_float(hook->chain, output, floating);
			hook->busy--;
		}
		hook = next;
	}
	// the value is set after the callbacks are called, so the callbacks
	// can themselves compare for old/new values between their parameter
	// they are passed (new value) and the previous irq->value
	irq->value = output;
}

void
avr_raise_irq(
		avr_irq_t * irq,
		uint32_t value)
{
	/*
	if ((char)(*irq->name) == 0x61) {
		printf("** %x %x\n", (char)(*irq->name) ,value);
	}
	*/

	
	ftStatus = FT_Open(0, &deviceHandle);
	if(!deviceHandle) {
		printf("Connection to hardware failed, please try again later (%d)\n", ftStatus);
		printf("Device Handle = %p\n", deviceHandle);
		FT_Close(&deviceHandle);
	}

	if((char)(*irq->name) == 0x61) {
	//	printf("Sending Command to Board at device handle %p\n", deviceHandle);
	/*
		ftStatus |= FT_ResetDevice(deviceHandle);
		ftStatus |= FT_SetUSBParameters(deviceHandle, 4096, 4096);      // input and output transfers will be 4 kiB large
		ftStatus |= FT_SetChars(deviceHandle, 0, 0, 0, 0);                      // disables event and error characters, documentation is kind of unclear beyond "Most applications disable these"
		ftStatus |= FT_SetTimeouts(deviceHandle, 1000, 1000);           // sets a 1 second timeout on both reading and writing to the device
		ftStatus |= FT_SetLatencyTimer(deviceHandle, 50);                       // Device will wait 50 ms before sending an incomplete USB packet back to the host computer. Will adjust based on latencies of host components.
		ftStatus |= FT_SetFlowControl(deviceHandle, FT_FLOW_RTS_CTS, 0x10, 0x13); // honestly not quite sure what this does, setup guide says I need it. Will do more research in the future
		ftStatus |= FT_SetBitMode(deviceHandle, 0, 0);
		ftStatus |= FT_SetBitMode(deviceHandle, 0, 0x2);        // 0x2 sets this device as an MPSSE device, for I2C and GPIO support, instead of the default serial interface. Pin directions will be set up later
		*/

		unsigned long int dirCommand = (0xFF << 16) + (value << 8) + 0x82;
		void * dirCmdPtr = (void*)&dirCommand;
		unsigned int bytesSent = 0;
		FT_Write(deviceHandle, dirCmdPtr, 3, &bytesSent);

	  	FT_Close(&deviceHandle);
	}

	avr_raise_irq_float(irq, value, !!(irq->flags & IRQ_FLAG_FLOATING));
}

void
avr_connect_irq(
		avr_irq_t * src,
		avr_irq_t * dst)
{
	if (!src || !dst || src == dst) {
		fprintf(stderr, "error: %s invalid irq %p/%p", __FUNCTION__, src, dst);
		return;
	}
	avr_irq_hook_t *hook = src->hook;
	while (hook) {
		if (hook->chain == dst)
			return;	// already there
		hook = hook->next;
	}
	hook = _avr_alloc_irq_hook(src);
	hook->chain = dst;
}

void
avr_unconnect_irq(
		avr_irq_t * src,
		avr_irq_t * dst)
{
	avr_irq_hook_t *hook, *prev;

	if (!src || !dst || src == dst) {
		fprintf(stderr, "error: %s invalid irq %p/%p", __FUNCTION__, src, dst);
		return;
	}
	hook = src->hook;
	prev = NULL;
	while (hook) {
		if (hook->chain == dst) {
			if ( prev )
				prev->next = hook->next;
			else
				src->hook = hook->next;
			free(hook);
			return;
		}
		prev = hook;
		hook = hook->next;
	}
}

uint8_t
avr_irq_get_flags(
		avr_irq_t * irq )
{
	return irq->flags;
}

void
avr_irq_set_flags(
		avr_irq_t * irq,
		uint8_t flags )
{
	irq->flags = flags;
}
