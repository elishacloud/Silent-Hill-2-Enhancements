//-------------------------------------------
// file server module
#pragma once

#define ADX_SERVER_ENABLE		1

void server_create();
void server_destroy();

void ADX_lock_init();
void ADX_lock_close();
void ADX_lock();
void ADX_unlock();
