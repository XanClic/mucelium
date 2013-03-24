#!/usr/bin/env ruby
# coding: utf-8

target = 'i586-pc-muxomucota'
prefix = Dir.pwd

ENV['TARGET'] = target
ENV['PREFIX'] = prefix

system('mkdir -p tmp bin lib usr/bin usr/lib')

Dir.chdir('tmp')
system('mkdir -p build-binutils build-gcc')

if !File.file?("#{prefix}/bin/#{target}-ld")
    if !File.directory?("binutils-2.23.1")
        if !File.file?("binutils-2.23.1.tar.bz2")
            puts("\033[1mLade binutils...\033[0m\n\n")
            system("wget http://ftp.gnu.org/gnu/binutils/binutils-2.23.1.tar.bz2") or exit 1
            puts("\n\033[1mBinutils geladen.\033[0m")
        end

        puts("\033[1mEntpacke binutils...\033[0m")
        system("tar xjf binutils-2.23.1.tar.bz2") or exit 1

        puts("\033[1mPatche binutils...\033[0m")
        system("patch -p0 < ../patches/binutils.patch") or exit 1
    end

    puts("\033[1mKonfiguriere binutils...\033[0m")

    Dir.chdir('build-binutils')
    system("../binutils-2.23.1/configure --target=#{target} '--prefix=#{prefix}'") or exit 1
    puts("\033[1mBaue binutils...\033[0m")
    system('make') or exit 1
    puts("\033[1mInstalliere binutils...\033[0m")
    system('make install') or exit 1

    ENV['PATH'] += ":#{prefix}/bin"

    Dir.chdir('..')
end

if !File.file?("#{prefix}/bin/#{target}-gcc")
    if !File.directory?("gcc-4.8.0")
        if !File.file?("gcc-4.8.0.tar.bz2")
            puts("\033[1mLade GCC...\033[0m\n\n")
            system("wget ftp://ftp.fu-berlin.de/unix/languages/gcc/releases/gcc-4.8.0/gcc-4.8.0.tar.bz2") or exit 1
            puts("\n\033[1mGCC geladen.\033[0m")
        end

        puts("\033[1mEntpacke GCC...\033[0m")
        system("tar xjf gcc-4.8.0.tar.bz2") or exit 1

        puts("\033[1mPatche GCC...\033[0m")
        system("patch -p0 < ../patches/gcc.patch") or exit 1
    end

    puts("\033[1mKonfiguriere GCC...\033[0m")

    Dir.chdir('build-gcc')
    system("../gcc-4.8.0/configure --target=#{target} '--prefix=#{prefix}' --disable-nls --enable-languages=c") or exit 1
    puts("\033[1mBaue GCC...\033[0m")
    system('make all-gcc') or exit 1
    puts("\033[1mInstalliere GCC...\033[0m")
    system('make install-gcc') or exit 1

    Dir.chdir('..')
end
