#include <Brick/Entity.hpp>
#include <Brick/Component.hpp>
#include <Brick/Hub.hpp>
#include <Stick/Test.hpp>

using namespace stick;
using namespace brick;

struct Vec3f
{
    ~Vec3f()
    {
    }

    Float32 x, y, z;
};

const Suite spec[] =
{
    SUITE("Basic Tests")
    {
        using Position = Component<ComponentName("Position"), Vec3f>;
        using Velocity = Component<ComponentName("Velocity"), Vec3f>;
        using Name = Component<ComponentName("Name"), String>;

        EXPECT(Name::name() == "Name");
        Hub hub;
        Entity inv;
        EXPECT(!inv.isValid());
        Entity a = hub.createEntity();
        EXPECT(a.isValid());
        EXPECT(hub.entityCount() == 1);
        EXPECT(!a.hasComponent<Position>());
        a.set<Position>(1.0f, 2.0f, 3.0f);
        EXPECT(a.hasComponent<Position>());
        EXPECT(a.get<Position>().x == 1.0f);
        EXPECT(a.get<Position>().y == 2.0f);
        EXPECT(a.get<Position>().z == 3.0f);
        a.set<Velocity>((Vec3f) {1.0f, 2.0f, 3.0f});
        EXPECT(a.hasComponent<Velocity>());
        a.removeComponent<Velocity>();
        EXPECT(!a.hasComponent<Velocity>());
        a.set<Name>("Something");
        EXPECT(a.hasComponent<Name>());
        EXPECT(a.maybe<Name>().ensure() == "Something");

        Entity b = hub.createEntity();
        EXPECT(!b.hasComponent<Name>());
        EXPECT(!b.hasComponent<Position>());
        EXPECT(!b.hasComponent<Velocity>());
        EXPECT(hub.entityCount() == 2);
        b.destroy();
        EXPECT(!b.isValid());
        EXPECT(hub.entityCount() == 1);

        Entity c = hub.createEntity();
        EXPECT(hub.entityCount() == 2);
        c.set<Position>(10.0f, 12.3f, 100.9f);
        c.set<Velocity>(12.3f, -13.2f, 2.0f);

        Entity d = hub.createEntity();
        d.set<Position>(-3.0f, 1.3f, 93.2f);
        d.set<Name>("Blubber");

        int visited = 0;
        DynamicArray<Vec3f> expectedPositions = {{1.0f, 2.0f, 3.0f}, { -3.0f, 1.3f, 93.2f}};
        DynamicArray<String> expectedNames = {"Something", "Blubber"};
        for (Entity e : hub.view<Position, Name>())
        {
            const Vec3f & pos = e.maybe<Position>().ensure();
            EXPECT(e.hasComponent<Name>());
            EXPECT(e.hasComponent<Position>());
            //printf("%s  %f, %f, %f\n", "", pos.x, pos.y, pos.z);
            auto it = findIf(expectedPositions.begin(), expectedPositions.end(), [&](const Vec3f & _vec) { return e.get<Position>().x == _vec.x && e.get<Position>().y == _vec.y && e.get<Position>().z == _vec.z; });
            EXPECT(it != expectedPositions.end());
            expectedPositions.remove(it);

            auto it2 = findIf(expectedNames.begin(), expectedNames.end(), [&](const String & _name) { return e.get<Name>() == _name; });
            EXPECT(it2 != expectedNames.end());
            expectedNames.remove(it2);

            visited++;
        }
        EXPECT(visited == 2);

        Entity e = d;
        EXPECT(e.isValid());
        EXPECT(e == d);
        EXPECT(e != a);
        d.destroy();
        EXPECT(!e.isValid());
    },
    SUITE("Clone Tests")
    {
        using Position = Component<ComponentName("Position"), Vec3f>;
        using Velocity = Component<ComponentName("Velocity"), Vec3f>;
        using Name = Component<ComponentName("Name"), String>;

        Hub hub;
        Entity a = hub.createEntity();
        a.set<Name>("Eggbert");
        a.set<Position>(1.0f, 2.0f, 3.0f);
        a.set<Velocity>(0.3f, 0.2f, 0.1f);

        Entity b = a.clone();
        EXPECT(b.hasComponent<Name>());
        EXPECT(b.hasComponent<Position>());
        EXPECT(b.hasComponent<Velocity>());
        EXPECT(b.get<Name>() == "Eggbert");
        EXPECT(b.get<Position>().x == 1.0f);
        EXPECT(b.get<Position>().y == 2.0f);
        EXPECT(b.get<Position>().z == 3.0f);
        EXPECT(b.get<Velocity>().x == 0.3f);
        EXPECT(b.get<Velocity>().y == 0.2f);
        EXPECT(b.get<Velocity>().z == 0.1f);

        Entity c = a.cloneWithout<Velocity>();
        EXPECT(c.hasComponent<Name>());
        EXPECT(c.hasComponent<Position>());
        EXPECT(!c.hasComponent<Velocity>());

        using Pivot = Component<ComponentName("Pivot"), Vec3f>;
        a.set<Pivot>(101.0f, 201.0f, 303.0f);
        Entity d = a.cloneWithout<Velocity, Name>();
        EXPECT(!d.hasComponent<Name>());
        EXPECT(d.hasComponent<Position>());
        EXPECT(!d.hasComponent<Velocity>());
        EXPECT(d.hasComponent<Pivot>());
        EXPECT(d.get<Pivot>().x == 101.0f);
        EXPECT(d.get<Pivot>().y == 201.0f);
        EXPECT(d.get<Pivot>().z == 303.0f);

        Entity e = a.cloneWith<Velocity, Name>();
        EXPECT(e.hasComponent<Name>());
        EXPECT(!e.hasComponent<Position>());
        EXPECT(e.hasComponent<Velocity>());
        EXPECT(!e.hasComponent<Pivot>());
    }
};

int main(int _argc, const char * _args[])
{
    return runTests(spec, _argc, _args);
}
