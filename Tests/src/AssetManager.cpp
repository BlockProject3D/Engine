// Copyright (c) 2020, BlockProject
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution.
//     * Neither the name of BlockProject nor the names of its contributors
//       may be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#define BP_COMPAT_2_X
#include "ListLogHandler.hpp"
#include <Engine/AssetManager.hpp>
#include <Engine/SimpleAsset.hpp>
#include <gtest/gtest.h>

using namespace bp3d;

class DummyProvider final : public IAssetProvider
{
public:
    bpf::memory::UniquePtr<IAssetBuilder> Create(const bpf::String &) final
    {
        return (nullptr);
    }
};

class ExceptionProvider final : public IAssetProvider
{
public:
    bpf::memory::UniquePtr<IAssetBuilder> Create(const bpf::String &) final
    {
        throw bpf::RuntimeException("Well", "Too bad I failed...");
    }
};

class ExceptionBuilder final : public SimpleAsset
{
public:
    void Build() final
    {
        throw bpf::RuntimeException("Well", "This is definately a failure!");
    }

    bpf::memory::UniquePtr<Asset> Mount(AssetManager &, const bpf::String &) final
    {
        return (nullptr);
    }
};

class SuperBuilder final : public IAssetBuilder
{
private:
    bpf::String _loc;
    bpf::collection::List<bpf::Tuple<bpf::String, bpf::String>> _expanded;
    bpf::collection::List<bpf::Name> _emptyDependencies;

public:
    explicit SuperBuilder(const bpf::String &loc)
        : _loc(loc)
    {
    }

    void Build() final
    {
        bpf::system::Thread::Sleep(100); //Simulate long running work
        if (_loc.EndsWith("test.null"))
            _expanded.Add(bpf::Tuple<bpf::String, bpf::String>("Whatever", "bp3d::Asset/null,%Assets%/whatever.null"));
    }

    inline const bpf::collection::List<bpf::Tuple<bpf::String, bpf::String>> &GetExpandedAssets() const noexcept final
    {
        return (_expanded);
    }

    inline const bpf::collection::List<bpf::Name> &GetDependencies() const noexcept final
    {
        return (_emptyDependencies);
    }

    bpf::memory::UniquePtr<Asset> Mount(AssetManager &, const bpf::String &vpath) final
    {
        return (bpf::memory::MakeUnique<Asset>(bpf::Name(bpf::TypeName<Asset>()), vpath));
    }
};

class SuperProvider final : public IAssetProvider
{
private:
    bpf::system::Paths _paths;
    bool _except_;

public:
    SuperProvider(const bpf::system::Paths &paths, bool except)
        : _paths(paths)
        , _except_(except)
    {
    }
    bpf::memory::UniquePtr<IAssetBuilder> Create(const bpf::String &loc) final
    {
        if (_except_)
            return (bpf::memory::MakeUnique<ExceptionBuilder>());
        return (bpf::memory::MakeUnique<SuperBuilder>(loc));
    }
};

TEST(AssetManager, Basic)
{
    bpf::collection::List<bpf::String> logs;
    AssetManager manager;

    manager.AddLogHandler(bpf::memory::MakeUnique<ListLogHandler>(logs));
    manager.Add("Test/Null", "invalid");
    EXPECT_STREQ(*logs.First(), "[INFO]AssetManager> Loading asset 'Test/Null' with url 'invalid'...");
    EXPECT_STREQ(*logs.Last(), "[ERROR]AssetManager> Could not load asset 'Test/Null': incorrect asset url format");
}

TEST(AssetManager, NoProvider)
{
    bpf::collection::List<bpf::String> logs;
    AssetManager manager;

    manager.AddLogHandler(bpf::memory::MakeUnique<ListLogHandler>(logs));
    manager.Add("Test/Null", "bp3d::asset::Texture/bpx,%Assets%/test.bpx");
    EXPECT_STREQ(*logs.First(), "[INFO]AssetManager> Loading asset 'Test/Null' with url 'bp3d::asset::Texture/bpx,%Assets%/test.bpx'...");
    EXPECT_STREQ(*logs.Last(), "[ERROR]AssetManager> Could not load asset 'Test/Null': no installed provider matches asset format (bp3d::asset::Texture/bpx)");
}

TEST(AssetManager, Provider_Failure)
{
    bpf::collection::List<bpf::String> logs;
    AssetManager manager;

    manager.SetProvider<Asset>("null", bpf::memory::MakeUnique<DummyProvider>());
    manager.AddLogHandler(bpf::memory::MakeUnique<ListLogHandler>(logs));
    manager.Add("Test/Null", "bp3d::Asset/null,%Assets%/test.null");
    EXPECT_STREQ(*logs.First(), "[INFO]AssetManager> Loading asset 'Test/Null' with url 'bp3d::Asset/null,%Assets%/test.null'...");
    EXPECT_STREQ(*logs.Last(), "[ERROR]AssetManager> Could not load asset 'Test/Null': IAssetProvider failure");
}

TEST(AssetManager, Provider_Exception)
{
    bpf::collection::List<bpf::String> logs;
    AssetManager manager;

    manager.SetProvider<Asset>("null", bpf::memory::MakeUnique<ExceptionProvider>());
    manager.AddLogHandler(bpf::memory::MakeUnique<ListLogHandler>(logs));
    manager.Add("Test/Null", "bp3d::Asset/null,%Assets%/test.null");
    EXPECT_STREQ(*logs[0], "[INFO]AssetManager> Loading asset 'Test/Null' with url 'bp3d::Asset/null,%Assets%/test.null'...");
    EXPECT_STREQ(*logs[1], "[ERROR]AssetManager> Could not load asset 'Test/Null': an unhandled exception has occured");
    EXPECT_STREQ(*logs[2], "[ERROR]AssetManager>         > WellException: Too bad I failed...");
}

TEST(AssetManager, Builder_Exception)
{
    bpf::collection::List<bpf::String> logs;
    AssetManager manager;

    manager.SetProvider<Asset>("null", bpf::memory::MakeUnique<SuperProvider>(bpf::system::Paths(bpf::io::File(), bpf::io::File(), bpf::io::File(), bpf::io::File()), true));
    manager.AddLogHandler(bpf::memory::MakeUnique<ListLogHandler>(logs));
    manager.Add("Test/Null", "bp3d::Asset/null,%Assets%/test.null");
    EXPECT_STREQ(*logs[0], "[INFO]AssetManager> Loading asset 'Test/Null' with url 'bp3d::Asset/null,%Assets%/test.null'...");
    manager.WaitForAllObjects();
    EXPECT_STREQ(*logs[1], "[ERROR]AssetManager> Could not build asset 'Test/Null': an unhandled exception has occured");
    EXPECT_STREQ(*logs[2], "[ERROR]AssetManager>         > WellException: This is definately a failure!");
}

TEST(AssetManager, Builder)
{
    bpf::collection::List<bpf::String> logs;
    AssetManager manager;

    manager.SetProvider<Asset>("null", bpf::memory::MakeUnique<SuperProvider>(bpf::system::Paths(bpf::io::File(), bpf::io::File(), bpf::io::File(), bpf::io::File()), false));
    manager.AddLogHandler(bpf::memory::MakeUnique<ListLogHandler>(logs));
    manager.Add("Test/Null", "bp3d::Asset/null,%Assets%/test.null");
    EXPECT_STREQ(*logs[0], "[INFO]AssetManager> Loading asset 'Test/Null' with url 'bp3d::Asset/null,%Assets%/test.null'...");
    manager.WaitForAllObjects();
    EXPECT_STREQ(*logs[1], "[INFO]AssetManager> Loading asset 'Test/Null/Whatever' with url 'bp3d::Asset/null,%Assets%/whatever.null'...");
    EXPECT_STREQ(*logs[2], "[INFO]AssetManager> Successfully loaded asset 'Test/Null'");
    EXPECT_STREQ(*logs[3], "[INFO]AssetManager> Successfully loaded asset 'Test/Null/Whatever'");
}

TEST(AssetManager, Get)
{
    bpf::collection::List<bpf::String> logs;
    AssetManager manager;

    manager.SetProvider<Asset>("null", bpf::memory::MakeUnique<SuperProvider>(bpf::system::Paths(bpf::io::File(), bpf::io::File(), bpf::io::File(), bpf::io::File()), false));
    manager.AddLogHandler(bpf::memory::MakeUnique<ListLogHandler>(logs));
    manager.Add("Test/Null", "bp3d::Asset/null,%Assets%/test.null");
    EXPECT_STREQ(*logs[0], "[INFO]AssetManager> Loading asset 'Test/Null' with url 'bp3d::Asset/null,%Assets%/test.null'...");
    manager.WaitForAllObjects();
    EXPECT_STREQ(*logs[1], "[INFO]AssetManager> Loading asset 'Test/Null/Whatever' with url 'bp3d::Asset/null,%Assets%/whatever.null'...");
    EXPECT_STREQ(*logs[2], "[INFO]AssetManager> Successfully loaded asset 'Test/Null'");
    EXPECT_STREQ(*logs[3], "[INFO]AssetManager> Successfully loaded asset 'Test/Null/Whatever'");
    EXPECT_NE(manager.Get<Asset>(bpf::Name("Test/Null")), nullptr);
    EXPECT_NE(manager.Get<Asset>(bpf::Name("Test/Null/Whatever")), nullptr);
    EXPECT_STREQ(*manager.Get<Asset>(bpf::Name("Test/Null"))->VirtualPath(), "Test/Null");
    EXPECT_STREQ(*manager.Get<Asset>(bpf::Name("Test/Null/Whatever"))->VirtualPath(), "Test/Null/Whatever");
    EXPECT_EQ(manager.Get<Asset>(bpf::Name()), nullptr);
    EXPECT_EQ(manager.Get<bpf::memory::Object>(bpf::Name("Test/Null")), nullptr); //Unrelated type with no default should return null
}
