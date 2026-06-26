prefix := "/"
working_dir := justfile_directory()

# CREATE A NEW APPLICATION FROM github template
create-app app:
    python scripts/create_app.py {{app}}

# BUILDS FOR THE APPLICATIONS RUNNING ON CONFIGURED REAL HARDWARE
build app specs="" clean="yes":
    python scripts/build.py --app {{app}} --specs {{quote(specs)}} --clean {{clean}} 

# QEMU BUILDS
build-qemu app specs="" clean="yes":
    python scripts/build.py --app {{app}} --specs {{quote(specs)}} --clean {{clean}} --qemu
    
# TESTS WITH TWISTER
test test_suite_root map_file tags="":
    python scripts/twister.py --root {{test_suite_root}} --tags {{quote(tags)}} --map-file {{map_file}}

test-qemu test_suite_root tags="":
    python scripts/twister.py --root {{test_suite_root}} --tags {{quote(tags)}} --qemu
    
# CLANG-TIDY
clang-tidy app specs="":    
    # Step 1 — build to get compile_commands.json (build with all conf files to get the most complete database)
    python scripts/build.py --app {{app}} --specs {{quote(specs)}} --clean "yes" --native_sim
    
    # Step 2 — filter the compile_commands.json file for compatibility with clang-tidy
    mkdir -p build_clang
    python3 scripts/filter_compile_commands.py build/compile_commands.json build_clang/compile_commands.json

    # Step 3 — run clang-tidy against the filtered database
    clang-tidy-22 -p build_clang {{working_dir}}/{{app}}/src/main.cpp --extra-arg=-v    

run-clang-tidy app specs="":
    python scripts/run_clang_tidy.py --app {{app}} --spec {{quote(specs)}} --wd {{working_dir}}