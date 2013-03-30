#!/usr/bin/env ruby
# coding: utf-8

if ARGV.empty?
    $stderr.puts('Usage: ./update_env.rb path/to/µxoµcota')
    exit 1
end

if !File.directory?(ARGV[0])
    $stderr.puts("Directory #{ARGV[0]} does not exist.")
    exit 1
end

target = 'i586-pc-muxomucota'
arch = 'i386'
platform = 'pc'

system("mkdir -p #{target}/lib #{target}/include")

system("cp '#{ARGV[0]}/src/lib/crt/crt.o' #{target}/lib/crt0.o") or exit 1
system("cp '#{ARGV[0]}/src/lib/libc.a'   #{target}/lib") or exit 1
system("cp '#{ARGV[0]}/src/lib/libm.a'   #{target}/lib") or exit 1
system("cp '#{ARGV[0]}/src/lib/libcdi.a' #{target}/lib") or exit 1

system("cp -r #{['common', "arch/#{arch}", "platform/#{platform}"].map { |d| "'#{ARGV[0]}/src/include/#{d}'/*" } * ' '} #{target}/include") or exit 1

system("echo | bin/#{target}-gcc -x c - -c -o #{target}/lib/crtbegin.o") or exit 1
system("echo | bin/#{target}-gcc -x c - -c -o #{target}/lib/crtend.o")   or exit 1
