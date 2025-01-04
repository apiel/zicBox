import { dump } from "./libs/core";

const myvar = [{ VIEW: "hello" }, ...[{HELLO: 'world'}]];
dump(myvar);