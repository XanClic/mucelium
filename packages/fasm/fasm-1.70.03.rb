# coding: utf-8

$pkg_ver = '1.70.03'

def pkg_download
    if !File.directory?('fasm')
        if !File.file?("fasm-#{$pkg_ver}.tar.gz")
            msg_status "Downloading FASM #{$pkg_ver}..."
            system("wget http://flatassembler.net/fasm-#{$pkg_ver}.tar.gz > /dev/null 2> wget_log") or failed_hence_suicide 'wget failed.'
            msg_done
        end

        msg_status "Unpacking fasm-#{$pkg_ver}.tar.gz..."
        system("tar xzf fasm-#{$pkg_ver}.tar.gz") or failed_hence_suicide 'tar xzf failed.'
        msg_done
    end
end

def pkg_build
    Dir.chdir('fasm')
    msg_status 'Building FASM...'
    system('fasm source/libc/fasm.asm fasm.o &> asm_log') or failed_hence_suicide 'Assembling failed.'
    link(['fasm.o'], 'fasm') or failed_hence_suicide 'Linking failed.'
    msg_done
    Dir.chdir('..')
end

def pkg_install
    system("cp fasm/fasm '#{$prefix}/bin'") or suicide 'Unable to copy fasm binary.'
end
