# WebSer

## Configuration File EBNF Grammar

```EBNF
(* Top level structure *)
Config = [WhiteSpace], [GlobalDirectives], {ServerBlock}+, [WhiteSpace];

(* Global directives *)
GlobalDirectives = {Directive};

(* Server block *)
ServerBlock = [WhiteSpace], "server", WhiteSpace, "{", [WhiteSpace],
              {Directive | LocationBlock},
              [WhiteSpace], "}", [WhiteSpace];

(* Location block *)
LocationBlock = [WhiteSpace], "location", WhiteSpace, 
                (LocationPath | LocationRegexPath), WhiteSpace, 
                "{", [WhiteSpace],
                {Directive},
                [WhiteSpace], "}", [WhiteSpace];

(* Location paths *)
LocationPath = Value;
LocationRegexPath = "~", {WhiteSpace}+, Value;

(* Directive structure *)
Directive = [WhiteSpace], DirectiveName, {(WhiteSpace, Value)}+, [WhiteSpace], ";", [WhiteSpace];

(* Directive names *)
DirectiveName = "listen" | "host" | "server_name" | "error_page" | "client_max_body_size" |
                "root" | "index" | "allowed_methods" | "autoindex" | "return" |
                "upload_store" | "cgi_pass" | "cgi_timeout" | "session_enable" | 
                "session_name" | "session_path" | "session_timeout" |
                "connection_timeout" | "error_log" | "access_log";

(* Value types *)
Value = Number | Size | String;

(* Numeric value *)
Number = Digit, {Digit};
Digit = "0" | "1" | ... | "9";

(* Size value with suffix *)
Size = Number, Suffix;
Suffix = "K" | "k" | "M" | "m" | "G" | "g";

(* String value *)
String = (Letter | Digit | SpecialChar), {Letter | Digit | SpecialChar};
Letter = "a" | "b" | ... | "z" | "A" | "B" | ... | "Z";
SpecialChar = "_" | "-" | "." | "/";

(* Comment *)
Comment = "#", {AnyChar - EOL}, EOL;
AnyChar = ? any character ?;
EOL = {? end of line character ? | "\n"};

(* Whitespace handling *)
WhiteSpace = {" " | "\t" | "\n" | "\r" | Comment};
```

