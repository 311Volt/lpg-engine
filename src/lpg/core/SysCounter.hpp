//
// Created by volt on 12/26/2024.
//

#ifndef LPG_ENGINE_SYSTEMCOUNTER_HPP
#define LPG_ENGINE_SYSTEMCOUNTER_HPP

#include <reflect>

namespace lpg {

    static constexpr double MasterSystemFrequency = 360.0;

    enum class SysFreq {
        Div0001_360Hz,
        Div0002_180Hz,
        Div0003_120Hz,
        Div0005_72Hz,
        Div0007_51Hz43,
        Div0011_32Hz73,
        Div0013_27Hz69,
        Div0019_18Hz95,
        Div0029_12Hz41,
        Div0037_9Hz73,
        Div0067_5Hz37,
        Div0131_2Hz75,
        Div0257_1Hz40,
        Div0521_1sec45,
        Div1031_2sec86,
        Div2053_5sec70,
        Div4099_11sec39,
        Div8201_22sec78,
        MAX
    };

    inline constexpr int SysFreqToDivision(SysFreq freq) {
        std::string_view name = reflect::enum_name(freq);
        return (name[3]-'0')*1000 + (name[4]-'0')*100 + (name[5]-'0')*10 + (name[6]-'0');
    }

    static_assert(SysFreqToDivision(SysFreq::Div0029_12Hz41) == 29);
    static_assert(SysFreqToDivision(SysFreq::Div8201_22sec78) == 8201);

    class SystemCounters {
    public:




    private:

        std::array<int, std::to_underlying(SysFreq::MAX)> counters {};
        void advanceCounters() {
            for (int i=0; i<counters.size(); ++i) {
                counters[i] = (counters[i] + 1) % SysFreqToDivision(static_cast<SysFreq>(i));
            }
        }
    };


}




#endif //LPG_ENGINE_SYSTEMCOUNTER_HPP
