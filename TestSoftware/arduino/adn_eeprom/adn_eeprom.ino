/*
 * EEPROM Read
 *
 * Reads the value of each byte of the EEPROM and prints it
 * to the computer.
 * This example code is in the public domain.
 */

#define EEPROM_CLEAR_EXIST

#include <EEPROM.h>
unsigned long t_before;
unsigned long t_after;
unsigned long t_perf_read;
unsigned long t_perf_write;
unsigned long t_perf_get1;
unsigned long t_perf_get100;
unsigned long t_perf_put1;
unsigned long t_perf_put100;
unsigned long t_perf_read_sum = 0;
unsigned long t_perf_write_sum = 0;
unsigned long t_perf_get1_sum = 0;
unsigned long t_perf_get100_sum = 0;
unsigned long t_perf_put1_sum = 0;
unsigned long t_perf_put100_sum = 0;

byte byte1 = 0;
byte byte100[100];
byte dump[102];
const int ave_count = 10;

struct MyObject {
  float field1;
  byte field2;
  char name[10];
};

void perf_write() {
  byte1 = 1;
  t_before = millis();
  EEPROM.write(0, byte1);
  t_after = millis();
  t_perf_write = t_after - t_before;
  Serial.print("write:");
  Serial.print(t_perf_write);
  Serial.println("ms");
}

void perf_write_ave() {
  t_perf_write_sum = 0;
  for(int i=0; i<ave_count; i++) {
    Serial.println("");
    Serial.print("count:");
    Serial.println(i+1);
    perf_write();
    t_perf_write_sum += t_perf_write;
  }
  Serial.println("");
  Serial.print("write_ave:");
  Serial.print(t_perf_write_sum / ave_count);
  Serial.println("ms");
}

void perf_read() {
  t_before = millis();
  EEPROM.read(0);
  t_after = millis();
  t_perf_read = t_after - t_before;
  Serial.print("read:");
  Serial.print(t_perf_read);
  Serial.println("ms");
}

void perf_read_ave() {
  t_perf_read_sum = 0;
  for(int i=0; i<ave_count; i++) {
    Serial.println("");
    Serial.print("count:");
    Serial.println(i+1);
    perf_read();
    t_perf_read_sum += t_perf_read;
  }
  Serial.println("");
  Serial.print("read_ave:");
  Serial.print(t_perf_read_sum / ave_count);
  Serial.println("ms");
}

void perf_put() {
  byte1 = 2;
  t_before = millis();
  EEPROM.put(1, byte1);
  t_after = millis();
  t_perf_put1 = t_after - t_before;
  Serial.print("put(1byte):");
  Serial.print(t_perf_put1);
  Serial.println("ms");

  memset(byte100, 0x03, sizeof(byte100));
  t_before = millis();
  EEPROM.put(2, byte100);
  t_after = millis();
  t_perf_put100 = t_after - t_before;
  Serial.print("put(100byte):");
  Serial.print(t_perf_put100);
  Serial.println("ms");
}

void perf_put_ave() {
  t_perf_put1_sum = 0;
  t_perf_put100_sum = 0;
  for(int i=0; i<ave_count; i++) {
    Serial.println("");
    Serial.print("count:");
    Serial.println(i+1);
    perf_put();
    t_perf_put1_sum += t_perf_put1;
    t_perf_put100_sum += t_perf_put100;
  }
  Serial.println("");
  Serial.print("put(1byte)_ave:");
  Serial.print(t_perf_put1_sum / ave_count);
  Serial.println("ms");
  Serial.print("put(100byte)_ave:");
  Serial.print(t_perf_put100_sum / ave_count);
  Serial.println("ms");
}


void perf_get() {
  t_before = millis();
  EEPROM.get(1, byte1);
  t_after = millis();
  t_perf_get1 = t_after - t_before;
  Serial.print("get(1byte):");
  Serial.print(t_perf_get1);
  Serial.println("ms");

  t_before = millis();
  EEPROM.get(2, byte100);
  t_after = millis();
  t_perf_get100 = t_after - t_before;
  Serial.print("get(100byte):");
  Serial.print(t_perf_get100);
  Serial.println("ms");
}

void perf_get_ave() {
  t_perf_get1_sum = 0;
  t_perf_get100_sum = 0;
  for(int i=0; i<ave_count; i++) {
    Serial.println("");
    Serial.print("count:");
    Serial.println(i+1);
    perf_get();
    t_perf_get1_sum += t_perf_get1;
    t_perf_get100_sum += t_perf_get100;
  }
  Serial.println("");
  Serial.print("get(1byte)_ave:");
  Serial.print(t_perf_get1_sum / ave_count);
  Serial.println("ms");
  Serial.print("get(100byte)_ave:");
  Serial.print(t_perf_get100_sum / ave_count);
  Serial.println("ms");
}

void dump_all() {
  EEPROM.get(0, dump);
  for(int i = 0; i < (int)(sizeof(dump)); i++) {
    if((i + 1) % 10 == 0) {
      Serial.println(dump[i], HEX);
    } else {
      Serial.print(dump[i], HEX);
      Serial.print(" ");
    }
  }
  Serial.println("");
}

#ifdef EEPROM_CLEAR_EXIST
void clear() {
  Serial.println("clear start");
  Serial.println("write all 1");
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 1);
  }

  Serial.println("verify all 1");
  for (int i = 0; i < EEPROM.length(); i++) {
    if(EEPROM.read(i) != 1) {
      Serial.println("verify all 1 fail!");
      return;
    }
  }

  Serial.println("run clear");
  EEPROM.clear();

  Serial.println("verify clear");
  for (int i = 0; i < EEPROM.length(); i++) {
    if(EEPROM.read(i) != 0) {
      Serial.println("verify clear fail!");
      return;
    }
  }
  Serial.println("clear ok");
}
#endif

void read_write() {
  Serial.println("read_write start");
  for(int i = 0; i < 100; i++) {
    EEPROM.write(i, i);
    if (EEPROM.read(i) != i) {
      Serial.println("read_write fail!");
    }
  }
  Serial.println("read_write ok");
}

void read_update() {
  Serial.println("read_update start");
  for(int i = 0; i < 100; i++) {
    EEPROM.update(i, i);
    if (EEPROM.read(i) != i) {
      Serial.println("read_update fail!");
    }
  }
  Serial.println("read_update ok");
}

void iterator() {
  Serial.println("iterator start");
  for(int i = 0; i < 100; i++) {
    EEPROM[i] = i;
    if(EEPROM[i] != i) {
      Serial.println("fail!");
      return;
    }
    EEPROM[i] += 1;
    if(EEPROM[i] != (i+1)) {
      Serial.println("fail!");
      return;
    }
  }
  Serial.println("iterator ok");
}

void get_put() {
  const MyObject customVar = {
    3.14f,
    65,
    "Working!"
  };
  const float f = 123.456f;
  int addr = 0;
  Serial.println("get_put start");
  Serial.println("put float");
  EEPROM.put(addr, f);
  addr += sizeof(float);

  Serial.println("put custom data");
  EEPROM.put(addr, customVar);

  addr = 0;
  float get_f = 0;
  Serial.println("get float");
  EEPROM.get(addr, get_f);
  Serial.println("verify get float");
  if(f != get_f) {
    Serial.println("verify get float fail!");
    return;
  }
  addr += sizeof(float);
  MyObject get_customVar;
  Serial.println("get custom data");
  EEPROM.get(addr, get_customVar);
  Serial.println("verify get custom data");
  if(customVar.field1 != get_customVar.field1) {
    Serial.println("customVar.field1 fail!");
    return;
  }
  if(customVar.field2 != get_customVar.field2) {
    Serial.println("customVar.field2 fail!");
    return;
  }
  if(strncmp(customVar.name, get_customVar.name, sizeof(customVar.name)) != 0) {
    Serial.println("customVar.name fail!");
    return;
  }
  Serial.println("get_put ok");
}

void perf_all() {
  init();
  perf_write();
  perf_read();
  perf_put();
  perf_get();
}

void perf_all_ave() {
  t_perf_read_sum = 0;
  t_perf_write_sum = 0;
  t_perf_get1_sum = 0;
  t_perf_get100_sum = 0;
  t_perf_put1_sum = 0;
  t_perf_put100_sum = 0;
  for(int i=0; i<ave_count; i++) {
    Serial.println("");
    Serial.print("count:");
    Serial.println(i+1);
    perf_all();
    t_perf_read_sum   += t_perf_read;
    t_perf_write_sum  += t_perf_write;
    t_perf_get1_sum   += t_perf_get1;
    t_perf_get100_sum += t_perf_get100;
    t_perf_put1_sum   += t_perf_put1;
    t_perf_put100_sum += t_perf_put100;
  }
  Serial.println("");
  Serial.println("average");
  Serial.print("write:");
  Serial.print(t_perf_write_sum / ave_count);
  Serial.println("ms");
  Serial.print("read:");
  Serial.print(t_perf_read_sum / ave_count);
  Serial.println("ms");
  Serial.print("put(1byte):");
  Serial.print(t_perf_put1_sum / ave_count);
  Serial.println("ms");
  Serial.print("put(100byte):");
  Serial.print(t_perf_put100_sum / ave_count);
  Serial.println("ms");
  Serial.print("get(1byte):");
  Serial.print(t_perf_get1_sum / ave_count);
  Serial.println("ms");
  Serial.print("get(100byte):");
  Serial.print(t_perf_get100_sum / ave_count);
  Serial.println("ms");
}

void init() {
  Serial.println("initializing...");
  memset(dump, 0x00, sizeof(dump));
  EEPROM.put(0, dump);
}

void setup() {
  // initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  init();
}

void loop() {
  while(Serial.available() > 0) {
    Serial.read();
  }

  Serial.println("input \"[command];\"");
  while(Serial.available() <= 0){}
  String t = Serial.readStringUntil(';');
  t.trim();  

  if(t.compareTo("perf_read") == 0) {
    perf_read();
  } else if(t.compareTo("perf_read_ave") == 0) {
    perf_read_ave();
  } else if(t.compareTo("perf_write") == 0) {
    perf_write();
  } else if(t.compareTo("perf_write_ave") == 0) {
    perf_write_ave();
  } else if(t.compareTo("perf_get") == 0) {
    perf_get();
  } else if(t.compareTo("perf_get_ave") == 0) {
    perf_get_ave();
  } else if(t.compareTo("perf_put") == 0) {
    perf_put();
  } else if(t.compareTo("perf_put_ave") == 0) {
    perf_put_ave();
  } else if(t.compareTo("init") == 0) {
    init();
  } else if(t.compareTo("dump") == 0) {
    dump_all();
  } else if(t.compareTo("perf_all") == 0) {
    perf_all();
  } else if(t.compareTo("perf_all_ave") == 0) {
    perf_all_ave();
#ifdef EEPROM_CLEAR_EXIST
  } else if(t.compareTo("clear") == 0) {
    clear();
#endif
  } else if(t.compareTo("read_write") == 0) {
    read_write();
  } else if(t.compareTo("read_update") == 0) {
    read_update();
  } else if(t.compareTo("iterator") == 0) {
    iterator();
  } else if(t.compareTo("get_put") == 0) {
    get_put();
  } else {
    Serial.println("unknown command");
  }
}
