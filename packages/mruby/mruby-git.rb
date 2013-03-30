# coding: utf-8

def pkg_download
    if !File.directory?('mruby')
        msg_status 'Cloning mruby from github...'
        system('git clone git://github.com/mruby/mruby.git > /dev/null 2> git_log') or failed_hence_suicide 'git-clone failed.'
        msg_done


        msg_status 'Adding cross compiling section to config script...'

        content = IO.read('mruby/build_config.rb') or failed_hence_suicide 'Could not read config script.'

        content = "# coding: utf-8\n\n" + content + <<-EOS

MRuby::CrossBuild.new('muxomucota') do |conf|
  toolchain :gcc

  conf.bins = %w(mrbc mruby mirb)

  conf.cc.command = "#{$cc}"
  conf.cc.flags << "#{$cflags}"
  conf.linker.command = "#{$link}"
  conf.linker.flags << "#{$linkflags}"

end
EOS

        IO.write('mruby/build_config.rb', content)

        msg_done
    end
end

def pkg_build
    Dir.chdir('mruby')
    msg_status 'Building mruby...'
    system('make > /dev/null 2> make_log') or failed_hence_suicide 'Make failed.'
    msg_done
    Dir.chdir('..')
end

def pkg_install
    system("cp mruby/build/muxomucota/bin/m{ruby,irb} '#{$prefix}/bin'") or suicide 'Unable to copy binaries.'
end
