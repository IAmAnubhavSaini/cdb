describe 'database' do
    def run_script(commands)
        raw_output = ""
        IO.popen("./build_db", "r+") do |pipe|
            commands.each do |command|
#                 puts "command", command
#                 raw_output += command + "\n"
                  pipe.puts command
            end
                raw_output = pipe.gets(nil)
        end
        raw_output.split("\n")
    end

    it 'inserts and retrieves a row' do
        result = run_script([
            "insert 1 user1 email@example.com password1",
            "select",
            ".exit",
        ])

        expect(result).to match_array([
            "",
            "COMMAND: insert ID(number) username(string) email(string) password(string)",
            "COMMAND: select",
            "COMMAND: .exit",
            "",
            "db > ",
            "done.",
            "db > [1, user1, email@example.com, password1]",
            "db > done.",
        ])
    end

    it 'prints error message when table is full' do
        script = (1..1991).map do |i|
            "insert #{i} user#{i} email#{i}@example.com password#{i}"
        end
        script << ".exit"
        result  = run_script(script)
        expect(result[-3]).to eq('db > Error: Table full.')
    end

    it 'allows inserting strings that are the maximum -1 length' do
        long_username = "a"*63
        long_email = "a"*255
        long_password = "a" * 255
        script = [
            "insert 1 #{long_username} #{long_email} #{long_password}",
            "select",
            ".exit",
        ]
        result = run_script(script)
        expect(result).to match_array([
             "",
            "COMMAND: insert ID(number) username(string) email(string) password(string)",
            "COMMAND: select",
            "COMMAND: .exit",
            "",
            "db > ",
            "db > [1, aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]",
            "done.",

            "db > done.",
        ])
    end

    it 'allows inserting strings that are the maximum length' do
        long_username = "a"*64
        long_email = "a"*256
        long_password = "a" * 256
        script = [
            "insert 1 #{long_username} #{long_email} #{long_password}",
            "select",
            ".exit",
        ]
        result = run_script(script)
        expect(result).to match_array([
             "",
            "COMMAND: insert ID(number) username(string) email(string) password(string)",
            "COMMAND: select",
            "COMMAND: .exit",
            "",
            "db > ",
            "db > [1, aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa, aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa]",
            "done.",

            "db > done.",
        ])
    end


    it 'prints error message for input that are above the maximum length' do
        long_username = "a"*65
        long_email = "a"*257
        long_password = "a" * 257
        script = [
            "insert 1 #{long_username} #{long_email} #{long_password}",
            "select",
            ".exit",
        ]
        result = run_script(script)
        expect(result).to match_array([
            "",
            "COMMAND: insert ID(number) username(string) email(string) password(string)",
            "COMMAND: select",
            "COMMAND: .exit",
            "",
            "db > Input error: String is too long.",
            "",
            "db > done.",
            "db > ",
        ])
    end
end
