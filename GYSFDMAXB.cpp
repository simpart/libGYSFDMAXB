#include "libGYSFDMAXB.h"
#include <vector>
#include <HardwareSerial.h>
#include <string.h>

/**
 * split string at delimiter
 * 
 * @param std::string [in] split target string
 * @param char [in] delimiter
 * @return std::vector<std::string> splitted list
 * @note private common function
 */
std::vector<std::string> split(std::string str, char del) {
    int first = 0;
    int last = str.find_first_of(del);
 
    std::vector<std::string> result;
 
    while (first < str.size()) {
        std::string subStr(str, first, last - first);
 
        result.push_back(subStr);
 
        first = last + 1;
        last = str.find_first_of(del, first);
 
        if (last == std::string::npos) {
            last = str.size();
        }
    }
 
    return result;
}

/**
 * for debug
 */
void dump_data (std::map<std::string, std::vector<std::string>> gps) {
  Serial.println("DUMP GPS DATA ===========");
  for (const auto& [key, value] : gps){
        Serial.print("  ");
        Serial.print(key.c_str());
        Serial.print(": ");
        for(std::string conts : value){
          Serial.print(conts.c_str());
          Serial.print(",");
        }
        Serial.println("");
  }

  Serial.println("=========================");
}

/**
 * get raw data from gps module
 * split data at '$'
 * 
 * @param char [out] charactor buffer
 * @param size_t [in] buffer size
 * @return bool true: got raw data
 *              false: there is no data in serial
 * @note private common function
 */
bool get_rawdata (char *out, size_t max_siz) {
  if (NULL == out) {
    return false;
  }

  bool get_flg = false;
  memset(out, 0x00, max_siz);
    
  for (int i=0; i < max_siz-1 ;i++) {
    
    if (Serial.available()) {
      get_flg = true;
      out[i] = Serial.read();
      
      if (0x24 == out[i]) {
        // '$' prefix
        out[i] = 0x00;
        break;
      }
    }
    
  }

  return get_flg;
}

/**
 * parse raw data, set gps data
 * 
 * @note private method
 */
void GYSFDMAXB::load_data (void) {
  try {
    std::string rcv_str;
    std::vector<std::string> split_recv;
    std::string data_index = "";
    std::vector<std::string> data_conts;
    char buff[256] = {0};

    /* clear the last data */
    gps_data.clear();
    
    while (get_rawdata(buff, sizeof buff)) {
      rcv_str    = buff;
      split_recv = split(rcv_str, ',');
      
      for (int idx=0; idx < split_recv.size() ;idx++) {
        
        if (0 == split_recv[idx].find("GP")) {
          /* data prefix 'GP-' */          
          if ((0 < data_index.length()) && (0 < data_conts.size())) {
            std::vector<std::string> set_conts;
            std::copy(data_conts.begin(), data_conts.end(), std::back_inserter(set_conts));
            /* regist gpsdatas ('GP-' key and values) to member */
            gps_data[data_index.c_str()] = set_conts;
          }
          data_index = split_recv[idx].c_str();
          data_conts.clear();
        } else {
          /* set data contents */
          data_conts.push_back(split_recv[idx]);
        }
        
      }
    }
  } catch (std::exception e) {
    gps_data.clear();
  }
}


/**
 * get current position 
 * 
 * @param PosInfo * [out] position data
 *         ->latitude["degrees"]
 *         ->latitude["minutes"]
 *         ->latitude["value"]
 *         ->longitude["degrees"]
 *         ->longitude["minutes"]
 *         ->longitude["value"]
 * @return bool true: successful
 *              false: failed or not positioned yet
 */
bool GYSFDMAXB::getpos (PosInfo *pos) {
  
  retry++;
  
  if ((NULL == pos) || (10 == retry)) {
    retry = 0;
    return false;
  }

  /* get data from gps module */
  //Serial.println("**************start load data");
  load_data();
  //Serial.println("**************end load data");

  if (true == getposGPGGA(pos)) {
    Serial.print("from GPGGA: ");
    retry = 0;
    return true;
  }
  
  if (true == getposGPRMC(pos)) {
    Serial.print("from GPRMC: ");
    retry = 0;
    return true;
  }
  
  if (true == getposGPGLL(pos)) {
    Serial.print("from GPGLL: ");
    retry = 0;
    return true;
  }

  delay(100);
  return getpos(pos);
}

/**
 * @note private method
 */
bool GYSFDMAXB::getposGPGGA (PosInfo *pos) {
  
  int gpgga_qos       = 5;
  int gpgga_latitude  = 1;
  int gpgga_longitude = 3;
  
  /* check 'GPGGA' key */
  if (0 == gps_data.count("GPGGA")) {
    return false;
  }
  
  std::vector<std::string> conts = gps_data["GPGGA"];
  /* check contents */
  if (gpgga_qos > conts.size()) {
      //Serial.println("****debug: could not find QOS contents");
      return false;
  }
  
  /* check quality */
  if ((0 == conts[gpgga_qos].length()) || (0 == std::stoi(conts[gpgga_qos]))) {
    //Serial.println("****debug: invalid qos value");
    return false;
  }
  
  /*** set position ***/
  int deg_buf = 0;
  float lati  = std::stof(conts[gpgga_latitude]);
  float longi = std::stof(conts[gpgga_longitude]);
  /* latitude */
  deg_buf = lati / 100;
  pos->setLatitude((float) deg_buf, lati - (deg_buf*100));
  
  /* logitude */
  deg_buf = longi / 100;
  pos->setLongitude((float) deg_buf, longi - (deg_buf*100));
  
  return true;
}

/**
 * @note private method
 */
bool GYSFDMAXB::getposGPRMC (PosInfo *pos) {
  int gprmc_sts       = 1;
  int gprmc_latitude  = 2;
  int gprmc_longitude = 4;
  
  /* check 'GPRMC' key */
  if (0 == gps_data.count("GPRMC")) {
    return false;
  }
  
  std::vector<std::string> conts = gps_data["GPRMC"];
  /* check contents */
  if (gprmc_sts > conts.size()) {
      return false;
  }
  
  /* check quality */
  if ((0 == conts[gprmc_sts].length()) || ("V" == conts[gprmc_sts])) {
    return false;
  }
  
  /*** set position ***/
  int deg_buf = 0;
  float lati  = std::stof(conts[gprmc_latitude]);
  float longi = std::stof(conts[gprmc_longitude]);
  /* latitude */
  deg_buf = lati / 100;
  pos->setLatitude((float) deg_buf, lati - (deg_buf*100));
  
  /* logitude */
  deg_buf = longi / 100;
  pos->setLongitude((float) deg_buf, longi - (deg_buf*100));

  return true;
}

/**
 * @note private method
 */
bool GYSFDMAXB::getposGPGLL (PosInfo *pos) {
  int gpgll_sts       = 5;
  int gpgll_latitude  = 0;
  int gpgll_longitude = 2;
  
  /* check 'GPGLL' key */
  if (0 == gps_data.count("GPGLL")) {
    return false;
  }
  
  std::vector<std::string> conts = gps_data["GPGLL"];
  /* check contents */
  if (gpgll_sts > conts.size()) {
      return false;
  }
  
  /* check quality */
  if ((0 == conts[gpgll_sts].length()) || ("V" == conts[gpgll_sts])) {
    return false;
  }
  
  /*** set position ***/
  int deg_buf = 0;
  float lati  = std::stof(conts[gpgll_latitude]);
  float longi = std::stof(conts[gpgll_longitude]);
  /* latitude */
  deg_buf = lati / 100;
  pos->setLatitude((float) deg_buf, lati - (deg_buf*100));
  
  /* logitude */
  deg_buf = longi / 100;
  pos->setLongitude((float) deg_buf, longi - (deg_buf*100));

  return true;
}
/* end of file */
