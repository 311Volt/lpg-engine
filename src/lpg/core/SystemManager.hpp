//
// Created by volt on 12/26/2024.
//

#ifndef LPG_ENGINE_SYSTEMMANAGER_HPP
#define LPG_ENGINE_SYSTEMMANAGER_HPP

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
        Div0017_21Hz18,
        Div0019_18Hz95,
        Div0023_15Hz65,
        Div0029_12Hz41,
        Div0031_11Hz61,
        Div0037_9Hz73,
        Div0067_5Hz37,
        Div0131_2Hz75,
        Div0257_1Hz40,
        Div0521_1sec45,
        Div1031_2sec86,
        Div2053_5sec70,
        Div4099_11sec39,
        Div8201_22sec78
    };

    class SystemManager {

    };


}




#endif //LPG_ENGINE_SYSTEMMANAGER_HPP
