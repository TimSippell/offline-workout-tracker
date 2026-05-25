#pragma once

#include <swt/database.h>
#include <swt/repository.h>

struct TestFixture {
    sf::Database db{":memory:"};
    sf::Repository repo{db};

    TestFixture() { db.migrate(); }
};
