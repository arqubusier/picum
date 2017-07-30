#include "Arduino.h"
#include "main.h"
#include <i2c_t3.h>
#include "key_codes.h"

#define DEBUG

const int UNASSIGNED_DOWN = -2;
const int UNASSIGNED_UP = -1;
const int HID_RELEASED = 0;

const uint8_t target_write = 0x4E;
const uint8_t target_read = 0x4F;
//NOTE: values valid when IOCON.BANK = 0 on MCP23018
const uint8_t IODIRA = 0x00;
const uint8_t IODIRB = 0x01;
const uint8_t GPPUA = 0x0C;
const uint8_t GPPUB = 0x0D;
const uint8_t GPIOA = 0x12;
const uint8_t GPIOB = 0x13;

const int HID_MAX = 256;


const int N_ROWS = 4;
const int N_COLS = 12;
const int N_OUT_PINS = 4;
const int N_IN_PINS = 6;
const int REMOTE_COL_OFFS = N_COLS/2;
const int N_OUT_REMOTES = 6;
const int N_IN_REMOTES = 4;
const int COL_PINS[] = {0, 1, 2, 3, 4, 5};
const int ROW_PINS[] = {6, 7, 8, 9};
const int LED_PIN = 13;

const int N_FN_LAYERS = 4;
const int DEFAULT_FN_LAYER = 0;
const int MAX_N_USB_KEYS = 6;

//fn levels 0, 1, 2, 3
int fn_l = 0;
int fn_r = 0;
int fn_layer = 0;
int n_pressed_keys = 0;

int_stack free_usb_keys;

//Counters for all modifiers showing how many keys affect them 
int modifier_counters[N_MODIFIERS] = {0};


#define TARGET 0x20 // target Slave0 address
#define IODIRA 0x00
#define IODIRB 0x01
#define GPIOA 0x12
#define GPIOB 0x13
#define GPPUA 0x0C
#define GPPUB 0x0D

int last_err = 0;

void i2c_init(){
    // Setup for Master mode, pins 29/30, external pullups, 400kHz, 200ms default timeout
    Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_29_30, I2C_PULLUP_EXT, 100000);
    Wire1.setDefaultTimeout(200000); // 200ms
}

void mcp23018_write_reg(const uint8_t target, const uint8_t addr, const uint8_t d_in){
    Wire1.beginTransmission(target);   // Slave address
    Wire1.write(addr);
    Wire1.write(d_in);
    Wire1.endTransmission();
}

uint8_t mcp23018_read_reg(const uint8_t target, const uint8_t addr){
    Wire1.beginTransmission(target);
    Wire1.write(addr);
    Wire1.endTransmission();
    Wire1.requestFrom(target, (size_t)1);
    uint8_t dout = 0x00;

    last_err = Wire1.getError();
    //Check if error occured
    if(last_err){
        return 0;
    }
    else
    {
        while (Wire1.available())
            dout = Wire1.readByte();
    }

    return dout;
}

int mcp23018_read_status(){
  return last_err;
}

void mcp23018_init(){
  mcp23018_write_reg(TARGET, IODIRB, 0xFF);
  mcp23018_write_reg(TARGET, IODIRA, 0x00);
  mcp23018_write_reg(TARGET, GPPUB, 0xFF);
  mcp23018_write_reg(TARGET, GPPUA, 0x00);
}

/*
  dvorak
  1234567890[]
  ',.pyfgcrl/=
  aoeuidhtns-\
  ;qjkxbmwvz
  swedish
  1234567890+´
  qwertyuiopå¨
  asdfghjklöä'
  <zxcvbnm,.-
  us
  ...
*/
key_data_t key_map[N_COLS*N_ROWS*N_FN_LAYERS] =
{
  //level 0
  {MODIFIER_NONE, KEY_SW_QUOTE}, {MODIFIER_NONE, KEY_SW_COMMA}, {MODIFIER_NONE, KEY_SW_PUNCT}, {MODIFIER_NONE, KEY_SW_P}, {MODIFIER_NONE, KEY_SW_Y}, {MODIFIER_NONE, KEY_SW_ENTER},
    {MODIFIER_NONE, KEY_ENTER}, {MODIFIER_NONE, KEY_SW_F}, {MODIFIER_NONE, KEY_SW_G}, {MODIFIER_NONE, KEY_SW_C}, {MODIFIER_NONE, KEY_SW_R}, {MODIFIER_NONE, KEY_SW_L},
  {MODIFIER_NONE, KEY_SW_A}, {MODIFIER_NONE, KEY_SW_O}, {MODIFIER_NONE, KEY_SW_E}, {MODIFIER_NONE, KEY_SW_U}, {MODIFIER_NONE, KEY_SW_I}, {MODIFIER_NONE, KEY_SW_SPACE},
    {MODIFIER_NONE, KEY_SW_D}, {MODIFIER_NONE, KEY_SW_H}, {MODIFIER_NONE, KEY_SW_T}, {MODIFIER_NONE, KEY_SW_N}, {MODIFIER_NONE, KEY_SW_S}, {MODIFIER_NONE, KEY_SW_HYPH},
  {MODIFIER_LSHIFT, KEY_COMMA}, {MODIFIER_NONE, KEY_SW_Q}, {MODIFIER_NONE, KEY_SW_J}, {MODIFIER_NONE, KEY_SW_K}, {MODIFIER_NONE, KEY_SW_X}, {MODIFIER_NONE, KEY_SW_BACKSPACE},
    {MODIFIER_NONE, KEY_SW_B}, {MODIFIER_NONE, KEY_SW_M}, {MODIFIER_NONE, KEY_SW_W}, {MODIFIER_NONE, KEY_SW_V}, {MODIFIER_NONE, KEY_SW_Z}, {MODIFIER_NONE, KEY_SW_ENTER},
  {0, 0}, {0, KEY_SW_LEFT}, {0, KEY_SW_UP}, {0, KEY_SW_RIGHT}, {0, 0}, {0, 0}, 
    {0, KEY_SW_DOWN}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
  //level 1
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
  //level 2
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
    //level 3
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
   //level 4
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
  {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 
    {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}
};

int key_status[N_COLS*N_ROWS];
unsigned int modifier;

//out corresponds to rows 0-4
//in corresponds to columns 0-5
unsigned int local_key_address(int out, int in){
  return N_COLS*out+in;
}


//out corresponds to columns 5-11
//in corresponds to rows 0-4
unsigned int remote_key_address(int out, int in){
  return N_COLS*out + REMOTE_COL_OFFS + in;
}

unsigned int key_address(int offs, int layer){
  return layer*N_COLS*N_ROWS + offs;
}

void set_int_array(int *arr, size_t n_elems, int val){
  size_t i=0;
  for (;i<n_elems;++i){
    arr[i] = val;
  }  
}

void reset_key_statuses(){
  set_int_array(key_status, N_COLS*N_ROWS, UNASSIGNED_UP);
}

void print_int_array(int *arr, size_t n_elems){
  size_t i=0;
  for (;i<n_elems;++i){
    Serial.print(arr[i]);
    Serial.print(", ");
  }
  Serial.println("");
}

void init_main() {
  Serial.println("Begin Init");
 
  //Led used for debugging
  pinMode(LED_PIN, OUTPUT);
  
  //start serial connection
  Serial.begin(38400);
  //pins 0, 1, 2, 3, 4, 5 correspond to columns for the half with the mcu.
  int r=0;
  for (;r<N_OUT_PINS;++r){
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], LOW);
  }

  int c = 0;
  for (;c<N_IN_PINS;++c){
    pinMode(COL_PINS[c], INPUT_PULLDOWN);
  }

  free_usb_keys.push(5);
  free_usb_keys.push(4);
  free_usb_keys.push(3);
  free_usb_keys.push(2);
  free_usb_keys.push(1);
  free_usb_keys.push(0);

  i2c_init();
  mcp23018_init();
  
  reset_key_statuses();
  set_int_array(modifier_counters, N_MODIFIERS, 0);
  modifier=0;
  Serial.println("Finish Init");
}


void write_matrix_local(int row){
    int i=0;
    int val = HIGH;
    for (;i<N_OUT_PINS;++i)
    {
      if (i == row)
        val = HIGH;
      else
        val = LOW;
      digitalWrite(ROW_PINS[i], val);
    }

}

void write_matrix_remote(int output){
  uint8_t output_mask = 0xFF;
  bitClear(output_mask, output);

  mcp23018_write_reg(TARGET, GPIOA, output_mask);
}

void read_matrix_remote(int *out_buf, size_t buf_len){
  uint8_t res = mcp23018_read_reg(TARGET, GPIOB);
  int err = mcp23018_read_status();

  if (err){
    #ifdef DEBUG
      Serial.print("Error receiving: ");
      Serial.println(err);
    #endif
    return;
  }
      
  #ifdef DEBUG
    Serial.printf("received from %#02x\n", TARGET);
    Serial.print(res, BIN);
    Serial.println(" OK");
  #endif
  
  int i;
  for (;i<buf_len; ++i){
    out_buf[i] = bitRead(i, res);
  }
}

void read_matrix_local(int *out_buff){
  int col=0;
  int col_pin=0;
  for (; col<N_IN_PINS; ++col){
    col_pin = COL_PINS[col];
    out_buff[col] = digitalRead(col_pin);
  }
}


int get_free_key_nr(){
  if (free_usb_keys.empty())
    return -1;
   
  return free_usb_keys.pop();
}

void return_key_nr(int key_nr){
  free_usb_keys.push(key_nr);
}

void send_usb(){
  Keyboard.send_now();
}

int get_key_status(int key_idx){
  return key_status[key_address(key_idx, 0)];
}

void set_status(int key_idx, int usb_key){
  key_status[key_address(key_idx, 0)] = usb_key;
}

key_data_t get_key_data(int key_idx){
  return key_map[key_address(key_idx,fn_layer)];
}

bool should_update(int key_state, int old_status){
  if ( 
      //Now: pressed, before: unassigned 
      (key_state == HIGH && old_status == UNASSIGNED_UP)
      //Now: up, before: pressed and assigned
      ||(key_state == LOW && old_status >= 0 )
     ){
    Serial.print("key_state ");
    Serial.print(key_state);
    Serial.print(" old status ");
    Serial.print(old_status);
    Serial.println(" 2");
    return true;
  }

  return false;
}

//TODO MAKE NON USB_KEYS DONT COUNT TOWARDS PRESSED KEYS
bool set_usb_key(int usb_key, int value){
  if (usb_key >= 0 && usb_key <= MAX_N_USB_KEYS
        && value<HID_MAX){
    keyboard_keys[usb_key] = value;
    return true;
  }
  return false;
}

unsigned int update_modifiers(int key_state, int modifier){
  unsigned int res = 0;
   
  if (modifier != MODIFIER_NONE){
    int counter = modifier_counters[modifier];
    if (key_state == HIGH){
      
      if (counter == 0){
        bitSet(res, modifier);
      }
        
      modifier_counters[modifier] = counter + 1;
    }
    else{
      if (counter == 1){
        bitClear(res,modifier);
      }
      modifier_counters[modifier] = counter - 1;
    }
  }

  Serial.println("MODIFIERS after");

  print_int_array(modifier_counters, N_MODIFIERS);
  Serial.println(res, BIN);
  return res;
}

void handle_special_keys(int key_state, key_data_t key_data){
  int key_value = key_data.hid_code;
  int layer = 0;
  
  if (key_value == KEY_FN_LAYER1){
    layer = 1;
  }
  else if (key_value == KEY_FN_LAYER2){
    layer = 2;
  }
  else if (key_value == KEY_FN_LAYER3){
    layer = 3;
  }
  else if (key_value == KEY_FN_LAYER4){
    layer = 4;
  }

  if (key_state == HIGH)
    fn_layer = layer;
  else
    fn_layer = DEFAULT_FN_LAYER;
}

void handle_key_press(int key_idx, int key_state){
      int key_nr = get_key_status(key_idx); //-1, 0, 1, 2, 3, 4, 5
      int next_key_nr = UNASSIGNED_UP;
      key_data_t key_data = get_key_data(key_idx);
      
      if (should_update(key_state, key_nr)){
        Serial.print("KEY_IDX ");
        Serial.print(key_idx);
        Serial.println(" B");
        int usb_key_value=0;
        bool should_update_usb = false;
        
        if (key_state == HIGH){
          Serial.println("KEY DOWN");
          key_nr = get_free_key_nr();
          next_key_nr = key_nr;
          usb_key_value = key_data.hid_code;
          ++n_pressed_keys;
        }
        else{
          Serial.println("KEY UP");
          return_key_nr(key_nr);     
          next_key_nr = UNASSIGNED_UP;
          usb_key_value = HID_RELEASED;
          --n_pressed_keys;
        }

        should_update_usb = set_usb_key(key_nr, usb_key_value);        
        Serial.print("key_nr ");
        Serial.print(key_nr);
        Serial.print(" usb_key ");
        Serial.print(usb_key_value);
        Serial.println(" 3");

        unsigned int new_modifier = update_modifiers(key_state, key_data.modifier);
        
        Serial.print("new_modifier ");
        Serial.print(new_modifier, BIN);
        Serial.print("modifier");
        Serial.println(modifier, BIN);
        if (new_modifier != modifier){
          Serial.println("Update modifier");
          should_update_usb = true;
          modifier=new_modifier;
          Keyboard.set_modifier(new_modifier);
        }

        handle_special_keys(key_state, key_data);
        
        if (should_update_usb){
          send_usb(); 
        }

        Serial.print("next status");
        Serial.println(next_key_nr);
        set_status(key_idx, next_key_nr);
      }
}

void run_main() {

  //TODO CALCULATE PROPER MAX AT COMPILE TIME
  const size_t OUT_LEN = N_OUT_PINS + N_OUT_REMOTES;
  
  int out_buff[OUT_LEN] = {0};
  set_int_array(out_buff, OUT_LEN, LOW);
  
  int out = 0;
  for (out=0;out<N_OUT_PINS;++out){
    write_matrix_local(out);
    read_matrix_local(out_buff);

    //print_int_array(out_buff, N_COLS);  
    int in = 0;
    for (;in<N_IN_PINS;++in){
      int key_idx = local_key_address(out, in);
      int key_val = out_buff[in];
      handle_key_press(key_idx, key_val);
    }
  }
}
