#!/usr/bin/env ruby
# coding: utf-8

def suicide msg
    puts("\033[0;31;1m#{msg}\033[0m")
    exit 1
end

def msg_status msg
    printf('%-70s', msg)
end

def msg_done
    puts("\033[0;32;1mdone\033[0m")
end

def msg_failed
    puts("\033[0;31;1mfailed\033[0m")
end

def failed_hence_suicide msg
    msg_failed
    suicide msg
end

def link(src, dst)
    system("#{$link} #{src.map { |f| "'#{f}'" } * ' '} -o '#{dst}' #{$linkflags}")
end


puts("µcelium")
puts("=======")

$hostlibgccdir = File.dirname(`gcc -m32 -print-libgcc-file-name`)

$target = 'i586-pc-muxomucota'
$prefix = Dir.pwd
$pprefix = "#{$prefix}/usr"

ENV['TARGET'] = $target
ENV['PREFIX'] = $prefix

ENV['PATH'] += ":#{$prefix}/bin:#{$prefix}/#{$target}/bin"

$cc = "#{$target}-gcc"
$link = $cc

$cflags = "-std=c11 -m32 '-L#{$hostlibgccdir}' -nostdinc '-I#{$prefix}/#{$target}/include' '-I#{$prefix}/include'"
$linkflags = "-m32 '-L#{$hostlibgccdir}' '-L#{$prefix}/#{$target}/lib' '-L#{$prefix}/lib' -lc -lgcc"


if ARGV.empty?
    suicide 'Usage: ./build.sh path/to/package.rb'
end


if File.file?(ARGV[0])
    $pbdir = File.expand_path(File.dirname(ARGV[0]))
    eval IO.read(ARGV[0])
else
    suicide "Build instructions #{ARGV[0]} not found."
end

Dir.chdir("#{$prefix}/tmp")
pkg_download
pkg_build
pkg_install
