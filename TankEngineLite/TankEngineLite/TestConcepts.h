#ifndef TEST_CONCEPTS_H
#define TEST_CONCEPTS_H

ECS::Universe* BuildUniverse()
{
    auto pUniverse = new ECS::Universe();
    auto pWorld = pUniverse->CreateWorld();
    
    // Populate the world with all the big systems
    // The order in which systems are put here also determines their order and distance
    //   to each other in memory
    // Systems cannot be rebuilt, it's important that these settings are well tested, 
    //  always start with bigger numbers, go down from that
    auto[pTCS, pPCS, pRCS, pAICS] = pWorld->BuildSystems<
        ECS::ComponentSystem<TransformComponent, 2048, 0, ECS::SystemExecution::SYNCHRONOUS>,
        ECS::ComponentSystem<PhysicsComponent, 2048, 1, ECS::SystemExecution::SYNCHRONOUS>,
        ECS::ComponentSystem<RenderComponent, 2048, 2, ECS::SystemExecution::SYNCHRONOUS>,
        ECS::ComponentSystem<AiComponent, 128, 3, ECS::SystemExecution::ASYNCHRONOUS>
    >();
    
    // Allow systemless componenents to still exist in general purpose component system
    pWorld->AllowSystemlessComponents(true); 
    
    return pUniverse;
}

#endif //!TEST_CONCEPTS_H
