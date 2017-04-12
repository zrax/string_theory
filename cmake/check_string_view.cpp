#ifdef TRY_EXPERIMENTAL_STRING_VIEW
#   include <experimental/string_view>
    namespace ST { using _std_string_view = std::experimental::string_view; }
#else
#   include <filesystem>
    namespace ST { using _std_string_view = std::string_view; }
#endif

int main(int argc, char *argv[])
{
    ST::_std_string_view p(argv[0]);
    return 0;
}
