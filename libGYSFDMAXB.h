#ifndef __LIBGYSFDMAX_H__
#define __LIBGYSFDMAX_H__

#include <map>
#include <vector>

class PosInfo {
  public:
    std::map<std::string, float> latitude;
    std::map<std::string, float> longitude;

    void setLatitude (float deg, float sec) {
      latitude["degrees"] = deg;
      latitude["minutes"] = sec;
      latitude["value"]   = (deg*100) + sec;
    }
    
    void setLongitude (float deg, float sec) {
      longitude["degrees"] = deg;
      longitude["minutes"] = sec;
      longitude["value"]   = (deg*100) + sec;
    }

    std::string dmmstr () {
      std::string ret = "";
      int buf = 0;

      /* set latitude */
      buf = (int) latitude["degrees"];
      ret.append(std::to_string(buf));
      ret.append(" ");
      ret.append(std::to_string(latitude["minutes"]));

      ret.append(",");
      
      /* set logitude */
      buf = (int) longitude["degrees"];
      ret.append(std::to_string(buf));
      ret.append(" ");
      ret.append(std::to_string(longitude["minutes"]));

      return ret;
    }
};

class GYSFDMAXB {
  private:
    int  retry = 0;
    void load_data (void);
    std::map<std::string, std::vector<std::string>> gps_data;
    bool getposGPGGA (PosInfo *);
    bool getposGPRMC (PosInfo *);
    bool getposGPGLL (PosInfo *);
    
  public:
    bool getpos (PosInfo *);

};

#endif
