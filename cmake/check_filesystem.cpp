#ifdef TRY_EXPERIMENTAL_FILESYSTEM
#   include <experimental/filesystem>
    namespace ST { namespace _filesystem = std::experimental::filesystem; }
#else
#   include <filesystem>
    namespace ST { namespace _filesystem = std::filesystem; }
#endif

int main(int argc, char *argv[])
{
    ST::_filesystem::path p(argv[0]);
    return 0;
}
