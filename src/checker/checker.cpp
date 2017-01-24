/*
Copyright (c) 2017 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Gabriel Ebner
*/
#include <fstream>
#include "kernel/init_module.h"
#include "util/init_module.h"
#include "util/test.h"
#include "util/sstream.h"
#include "util/sexpr/init_module.h"
#include "kernel/quotient/quotient.h"
#include "kernel/inductive/inductive.h"
#include "kernel/standard_kernel.h"
#include "checker/text_import.h"
#include "checker/simple_pp.h"

#if defined(LEAN_EMSCRIPTEN)
#include <emscripten.h>
#endif

using namespace lean;  // NOLINT

int main(int argc, char ** argv) {
#if defined(LEAN_EMSCRIPTEN)
    EM_ASM(
        var lean_path = process.env['LEAN_PATH'];
        if (lean_path) {
            ENV['LEAN_PATH'] = lean_path;
        }

        try {
            // emscripten cannot mount all of / in the vfs,
            // we can only mount subdirectories...
            FS.mount(NODEFS, { root: '/home' }, '/home');
            FS.mkdir('/root');
            FS.mount(NODEFS, { root: '/root' }, '/root');

            FS.chdir(process.cwd());
        } catch (e) {
            console.log(e);
        });
#endif

    if (argc < 2) {
        std::cout << "usage: leanchecker export.out lemma_to_print" << std::endl;
        return 1;
    }

    struct initer {
        initer() {
            save_stack_info();
            initialize_util_module();
            initialize_sexpr_module();
            initialize_kernel_module();
            initialize_inductive_module();
            initialize_quotient_module();
        }
        ~initer() {
            finalize_quotient_module();
            finalize_inductive_module();
            finalize_kernel_module();
            finalize_sexpr_module();
            finalize_util_module();
        }
    } initer;

    try {
        std::ifstream in(argv[1]);
        if (!in) throw exception(sstream() << "file not found: " << argv[1]);

        unsigned trust_lvl = 0;
        auto env = mk_environment(trust_lvl);
        env = declare_quotient(env);
        import_from_text(in, env);

        env.for_each_declaration([&] (declaration const & d) {
            if (d.is_axiom()) {
                std::cout << compose_many(
                        {format("axiom"), space(), simple_pp(d.get_name()), space(), format(":"), space(),
                         simple_pp(env, d.get_type()), line()});
            }
        });

        for (int i = 2; i < argc; i++) {
            name n = string_to_name(argv[i]);
            auto d = env.get(n);
            std::cout << compose_many(
                    {format("theorem"), space(), simple_pp(d.get_name()), space(), format(":"), space(),
                     simple_pp(env, d.get_type()), line()});
        }

        unsigned num_decls = 0;
        env.for_each_declaration([&] (declaration const &) { num_decls++; });
        std::cout << "checked " << num_decls << " declarations" << std::endl;

        return 0;
    } catch (throwable & ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    } catch (std::exception & ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}