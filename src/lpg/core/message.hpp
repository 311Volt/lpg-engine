//
// Created by volt on 12/22/2024.
//

#ifndef LPG_ENGINE_MESSAGE_HPP
#define LPG_ENGINE_MESSAGE_HPP


namespace lpg {

    struct PostSpawnMessage {

    };

    struct PreKillMessage {

    };

    struct UpdateMessage {
        double deltaTime;
    };

    struct TestMessage {
        const double a;
    };

}


#endif //MESSAGE_HPP
