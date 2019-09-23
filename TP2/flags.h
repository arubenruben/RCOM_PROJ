#pragma once



//  Campo de Controlo
#define SET 0x03
#define DISC 0x0B
#define UA 0x07
#define RR(n) ((n % 2) << 15 + 0x05)
#define REJ(n) ((n % 2) << 15 + 0x01)
