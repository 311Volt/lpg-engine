#include <iostream>

#include <entt/entt.hpp>
#include <assimp/Importer.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <lpg/lpg.hpp>


void say_hello(){

    btDefaultCollisionConfiguration btConf;
    entt::registry entt_registry;
    Assimp::Importer importer;

    printf("Hello, from lpg_engine!\n");

    lpg::Registry registry;
    struct {
        struct {
            al::Vec2i resolution = {800, 600};
            int depth = 32;
            float refreshRate = 60.0;
        } videomode;
        bool cheats = false;
        std::string nickname = "Player";
    } ex;
    registry.registerValue("root", ex);
    registry.dump(std::cout);


    std::vector<int> divisions;
    auto coprime_with_all = [&divisions](int n) {
        for (const auto& k: divisions) {
            if (n % k == 0) {
                return false;
            }
        }
        return true;
    };
    for (int i=2; i<10000;) {
        for (int j=0; j<i; j++) {
            if (coprime_with_all(i+j)) {
                divisions.push_back(i+j);
                break;
            }
        }
        if (i < 16) {
            i++;
        } else {
            i *= 2;
        }
    }
    // printf("divisions: %d\n\n", divisions.size());
    // for (auto& k: divisions) {
    //     printf("\tTimerDiv%04d_%.2f\n", k, std::max(360.0 / k, k / 360.0));
    // }
}
