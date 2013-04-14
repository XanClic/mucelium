class String
    def %(arg)
        if arg.kind_of?(Array)
            format(self, *arg)
        else
            format(self, arg)
        end
    end
end
