#include <gtest/gtest.h>

#include <dlib/hash.h>
#include "gamesys/resource.h"
#include "gamesys/test/test_resource_ddf.h"

class ResourceTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        factory = dmResource::NewFactory(16, ".");
    }

    virtual void TearDown()
    {
        dmResource::DeleteFactory(factory);
    }

    dmResource::HFactory factory;
};

dmResource::CreateResult DummyCreate(dmResource::HFactory factory, void* context, const void* buffer, uint32_t buffer_size, dmResource::SResourceDescriptor* resource)
{
    return dmResource::CREATE_RESULT_OK;
}

dmResource::CreateResult DummyDestroy(dmResource::HFactory factory, void* context, dmResource::SResourceDescriptor* resource)
{
    return dmResource::CREATE_RESULT_OK;
}

TEST_F(ResourceTest, RegisterType)
{
    dmResource::FactoryResult e;

    // Test create/destroy function == 0
    e = dmResource::RegisterType(factory, "foo", 0, 0, 0);
    ASSERT_EQ(dmResource::FACTORY_RESULT_INVAL, e);

    // Test dot in extension
    e = dmResource::RegisterType(factory, ".foo", 0, &DummyCreate, &DummyDestroy);
    ASSERT_EQ(dmResource::FACTORY_RESULT_INVAL, e);

    // Test "ok"
    e = dmResource::RegisterType(factory, "foo", 0, &DummyCreate, &DummyDestroy);
    ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);

    // Test already registred
    e = dmResource::RegisterType(factory, "foo", 0, &DummyCreate, &DummyDestroy);
    ASSERT_EQ(dmResource::FACTORY_RESULT_ALREADY_REGISTERED, e);
}

TEST_F(ResourceTest, NotFound)
{
    dmResource::FactoryResult e;
    e = dmResource::RegisterType(factory, "foo", 0, &DummyCreate, &DummyDestroy);
    ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);

    void* resource = (void*) 0xdeadbeef;
    e = dmResource::Get(factory, "DOES_NOT_EXISTS.foo", &resource);
    ASSERT_EQ(dmResource::FACTORY_RESULT_RESOURCE_NOT_FOUND, e);
    ASSERT_EQ((void*) 0, resource);
}

TEST_F(ResourceTest, UnknwonResourceType)
{
    dmResource::FactoryResult e;

    void* resource = (void*) 0;
    e = dmResource::Get(factory, "build/default/src/gamesys/test/test.testresourcecont", &resource);
    ASSERT_EQ(dmResource::FACTORY_RESULT_UNKNOWN_RESOURCE_TYPE, e);
    ASSERT_EQ((void*) 0, resource);
}

// Loaded version (in-game) of ResourceContainerDesc
struct TestResourceContainer
{
    uint64_t                                m_NameHash;
    std::vector<TestResource::ResourceFoo*> m_Resources;
};

dmResource::CreateResult ResourceContainerCreate(dmResource::HFactory factory,
                                                 void* context,
                                                 const void* buffer, uint32_t buffer_size,
                                                 dmResource::SResourceDescriptor* resource);

dmResource::CreateResult ResourceContainerDestroy(dmResource::HFactory factory, void* context, dmResource::SResourceDescriptor* resource);

dmResource::CreateResult FooResourceCreate(dmResource::HFactory factory,
                                           void* context,
                                           const void* buffer, uint32_t buffer_size,
                                           dmResource::SResourceDescriptor* resource);

dmResource::CreateResult FooResourceDestroy(dmResource::HFactory factory, void* context, dmResource::SResourceDescriptor* resource);

class GetResourceTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        m_ResourceContainerCreateCallCount = 0;
        m_ResourceContainerDestroyCallCount = 0;
        m_FooResourceCreateCallCount = 0;
        m_FooResourceDestroyCallCount = 0;

        m_Factory = dmResource::NewFactory(16, "build/default/src/gamesys/test/");
        m_ResourceName = "test.cont";

        dmResource::FactoryResult e;
        e = dmResource::RegisterType(m_Factory, "cont", this, &ResourceContainerCreate, &ResourceContainerDestroy);
        ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);

        e = dmResource::RegisterType(m_Factory, "foo", this, &FooResourceCreate, &FooResourceDestroy);
        ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);
    }

    virtual void TearDown()
    {
        dmResource::DeleteFactory(m_Factory);
    }

public:
    uint32_t           m_ResourceContainerCreateCallCount;
    uint32_t           m_ResourceContainerDestroyCallCount;
    uint32_t           m_FooResourceCreateCallCount;
    uint32_t           m_FooResourceDestroyCallCount;

    dmResource::HFactory m_Factory;
    const char*        m_ResourceName;
};

dmResource::CreateResult ResourceContainerCreate(dmResource::HFactory factory,
                                                 void* context,
                                                 const void* buffer, uint32_t buffer_size,
                                                 dmResource::SResourceDescriptor* resource)
{
    GetResourceTest* self = (GetResourceTest*) context;
    self->m_ResourceContainerCreateCallCount++;

    TestResource::ResourceContainerDesc* resource_container_desc;
    dmDDF::Result e = dmDDF::LoadMessage(buffer, buffer_size, &TestResource_ResourceContainerDesc_DESCRIPTOR, (void**) &resource_container_desc);
    if (e == dmDDF::RESULT_OK)
    {
        TestResourceContainer* resource_cont = new TestResourceContainer();
        resource_cont->m_NameHash = dmHashBuffer64(resource_container_desc->m_Name, strlen(resource_container_desc->m_Name));
        resource->m_Resource = (void*) resource_cont;

        for (uint32_t i = 0; i < resource_container_desc->m_Resources.m_Count; ++i)
        {
            TestResource::ResourceFoo* sub_resource;
            dmResource::FactoryResult factoy_e = dmResource::Get(factory, resource_container_desc->m_Resources[i], (void**)&sub_resource);
            assert( factoy_e == dmResource::FACTORY_RESULT_OK );
            resource_cont->m_Resources.push_back(sub_resource);
        }
        dmDDF::FreeMessage(resource_container_desc);
        return dmResource::CREATE_RESULT_OK;
    }
    else
    {
        return dmResource::CREATE_RESULT_UNKNOWN;
    }
}

dmResource::CreateResult ResourceContainerDestroy(dmResource::HFactory factory, void* context, dmResource::SResourceDescriptor* resource)
{
    GetResourceTest* self = (GetResourceTest*) context;
    self->m_ResourceContainerDestroyCallCount++;

    TestResourceContainer* resource_cont = (TestResourceContainer*) resource->m_Resource;

    std::vector<TestResource::ResourceFoo*>::iterator i;
    for (i = resource_cont->m_Resources.begin(); i != resource_cont->m_Resources.end(); ++i)
    {
        dmResource::Release(factory, *i);
    }
    delete resource_cont;
    return dmResource::CREATE_RESULT_OK;
}

dmResource::CreateResult FooResourceCreate(dmResource::HFactory factory,
                                           void* context,
                                           const void* buffer, uint32_t buffer_size,
                                           dmResource::SResourceDescriptor* resource)
{
    GetResourceTest* self = (GetResourceTest*) context;
    self->m_FooResourceCreateCallCount++;

    TestResource::ResourceFoo* resource_foo;

    dmDDF::Result e = dmDDF::LoadMessage(buffer, buffer_size, &TestResource_ResourceFoo_DESCRIPTOR, (void**) &resource_foo);
    if (e == dmDDF::RESULT_OK)
    {
        resource->m_Resource = (void*) resource_foo;
        resource->m_ResourceKind = dmResource::KIND_DDF_DATA;
        return dmResource::CREATE_RESULT_OK;
    }
    else
    {
        return dmResource::CREATE_RESULT_UNKNOWN;
    }
}

dmResource::CreateResult FooResourceDestroy(dmResource::HFactory factory, void* context, dmResource::SResourceDescriptor* resource)
{
    GetResourceTest* self = (GetResourceTest*) context;
    self->m_FooResourceDestroyCallCount++;

    dmDDF::FreeMessage(resource->m_Resource);
    return dmResource::CREATE_RESULT_OK;
}

TEST_F(GetResourceTest, GetTestResource)
{
    dmResource::FactoryResult e;

    TestResourceContainer* test_resource_cont = 0;
    e = dmResource::Get(m_Factory, m_ResourceName, (void**) &test_resource_cont);
    ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);
    ASSERT_NE((void*) 0, test_resource_cont);
    ASSERT_EQ(1, m_ResourceContainerCreateCallCount);
    ASSERT_EQ(0, m_ResourceContainerDestroyCallCount);
    ASSERT_EQ(test_resource_cont->m_Resources.size(), m_FooResourceCreateCallCount);
    ASSERT_EQ(0, m_FooResourceDestroyCallCount);
    ASSERT_EQ(123, test_resource_cont->m_Resources[0]->m_x);
    ASSERT_EQ(456, test_resource_cont->m_Resources[1]->m_x);

    ASSERT_EQ(dmHashBuffer64("Testing", strlen("Testing")), test_resource_cont->m_NameHash);
    dmResource::Release(m_Factory, test_resource_cont);
}

TEST_F(GetResourceTest, GetReference1)
{
    dmResource::FactoryResult e;

    dmResource::SResourceDescriptor descriptor;
    e = dmResource::GetDescriptor(m_Factory, m_ResourceName, &descriptor);
    ASSERT_EQ(dmResource::FACTORY_RESULT_NOT_LOADED, e);
}

TEST_F(GetResourceTest, GetReference2)
{
    dmResource::FactoryResult e;

    void* resource = (void*) 0;
    e = dmResource::Get(m_Factory, m_ResourceName, &resource);
    ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);
    ASSERT_NE((void*) 0, resource);
    ASSERT_EQ(1, m_ResourceContainerCreateCallCount);
    ASSERT_EQ(0, m_ResourceContainerDestroyCallCount);

    dmResource::SResourceDescriptor descriptor;
    e = dmResource::GetDescriptor(m_Factory, m_ResourceName, &descriptor);
    ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);
    ASSERT_EQ(1, m_ResourceContainerCreateCallCount);
    ASSERT_EQ(0, m_ResourceContainerDestroyCallCount);

    ASSERT_EQ(1, descriptor.m_ReferenceCount);
    dmResource::Release(m_Factory, resource);
}

TEST_F(GetResourceTest, ReferenceCountSimple)
{
    dmResource::FactoryResult e;

    TestResourceContainer* resource1 = 0;
    e = dmResource::Get(m_Factory, m_ResourceName, (void**) &resource1);
    ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);
    const uint32_t sub_resource_count = resource1->m_Resources.size();
    ASSERT_EQ(2, sub_resource_count); //NOTE: Hard coded for two resources in test.cont
    ASSERT_NE((void*) 0, resource1);
    ASSERT_EQ(1, m_ResourceContainerCreateCallCount);
    ASSERT_EQ(0, m_ResourceContainerDestroyCallCount);
    ASSERT_EQ(sub_resource_count, m_FooResourceCreateCallCount);
    ASSERT_EQ(0, m_FooResourceDestroyCallCount);

    dmResource::SResourceDescriptor descriptor1;
    e = dmResource::GetDescriptor(m_Factory, m_ResourceName, &descriptor1);
    ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);
    ASSERT_EQ(1, descriptor1.m_ReferenceCount);

    TestResourceContainer* resource2 = 0;
    e = dmResource::Get(m_Factory, m_ResourceName, (void**) &resource2);
    ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);
    ASSERT_NE((void*) 0, resource2);
    ASSERT_EQ(resource1, resource2);
    ASSERT_EQ(1, m_ResourceContainerCreateCallCount);
    ASSERT_EQ(0, m_ResourceContainerDestroyCallCount);
    ASSERT_EQ(sub_resource_count, m_FooResourceCreateCallCount);
    ASSERT_EQ(0, m_FooResourceDestroyCallCount);

    dmResource::SResourceDescriptor descriptor2;
    e = dmResource::GetDescriptor(m_Factory, m_ResourceName, &descriptor2);
    ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);
    ASSERT_EQ(2, descriptor2.m_ReferenceCount);

    // Release
    dmResource::Release(m_Factory, resource1);
    ASSERT_EQ(1, m_ResourceContainerCreateCallCount);
    ASSERT_EQ(0, m_ResourceContainerDestroyCallCount);
    ASSERT_EQ(sub_resource_count, m_FooResourceCreateCallCount);
    ASSERT_EQ(0, m_FooResourceDestroyCallCount);

    // Check reference count equal to 1
    e = dmResource::GetDescriptor(m_Factory, m_ResourceName, &descriptor1);
    ASSERT_EQ(dmResource::FACTORY_RESULT_OK, e);
    ASSERT_EQ(1, descriptor1.m_ReferenceCount);

    // Release again
    dmResource::Release(m_Factory, resource2);
    ASSERT_EQ(1, m_ResourceContainerCreateCallCount);
    ASSERT_EQ(1, m_ResourceContainerDestroyCallCount);
    ASSERT_EQ(sub_resource_count, m_FooResourceCreateCallCount);
    ASSERT_EQ(sub_resource_count, m_FooResourceDestroyCallCount);

    // Make sure resource gets unloaded
    e = dmResource::GetDescriptor(m_Factory, m_ResourceName, &descriptor1);
    ASSERT_EQ(dmResource::FACTORY_RESULT_NOT_LOADED, e);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

