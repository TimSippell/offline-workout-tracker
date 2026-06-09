#pragma once

#include <owt/database.h>
#include <owt/repository.h>

struct TestFixture {
    sf::Database db{":memory:"};
    sf::Repository repo{db};

    TestFixture() { db.migrate(); }
};
