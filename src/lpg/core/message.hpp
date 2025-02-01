//
// Created by volt on 12/22/2024.
//

#ifndef LPG_ENGINE_MESSAGE_HPP
#define LPG_ENGINE_MESSAGE_HPP

#include <reflect>
#include <stdexcept>
#include <unordered_map>
#include <string>

namespace lpg {

    struct MessageInterface {

        //TODO serialize & deserialize function pointers
    };

    class MessageRegistry {
    public:
        template<typename TMsg>
        int getMessageTypeId() {
            auto type_name = reflect::type_name<TMsg>();
            if (not messageTypeIdMap.contains(type_name)) {
                return -1;
            }
            return messageTypeIdMap.at(type_name);
        }

        template<typename TMsg>
        void registerMessageType() {
            auto type_name = reflect::type_name<TMsg>();
            if (messageTypeIdMap.contains(type_name)) {
                throw std::runtime_error("Message type already registered: " + std::string(type_name));
            }
            messageTypeIdMap.insert(reflect::type_name<TMsg>());
        }

    private:
        std::unordered_map<std::string, int> messageTypeIdMap; //TODO move to pimpl
        int idCounter = 0;
    };


    struct PostSpawnMessage {
        EntityDescriptor descriptor;
    };

    struct PreKillMessage {
        EntityDescriptor descriptor;
    };

    struct UpdateMessage {
        double deltaTime;
    };

    struct FixedUpdateMessage {
        double deltaTime;
    };

    struct TestMessage {
        const double a;
    };

}


#endif //MESSAGE_HPP
