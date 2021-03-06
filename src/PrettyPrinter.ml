(*module Print : sig
  val prog : Language.Prog.t -> unit
  val ints : int list -> unit
  val instrs : StackMachine.Instrs.t list -> unit
end =
  struct
    let (!) s = Printf.printf "%s" s
    let rec expr =
      let open Language.Expr in
      function
      | Const n -> Printf.printf "Const %d" n
      | Var x -> Printf.printf "Var %s" x
      | Binop (o, l, r) -> !"("; expr l; !(" " ^ o ^ " "); expr r; !")"
      | FCall (name, args) -> !name; !"("; List.iter (fun arg -> expr arg; !",") args; !")"
    let stmt st =
      let open Language.Stmt in
      let rec stmt' n =
        let doind n = for i = 1 to BatRef.get n do !" " done in
        function
        | Skip -> doind n; !"Skip "; !"\n"
        | Seq (l, r) -> stmt' n l; stmt' n r
        | Assign (x, e) -> doind n; !x; !" := "; expr e; !"\n"
        | While (e, s) -> doind n; !"While "; expr e; !" do"; !"\n"; n := (BatRef.get n) + 3; stmt' n s; n := (BatRef.get n) - 3; doind n; !"od"; !"\n"
        | If (e, s1, s2) -> doind n; !"If "; expr e; !" then\n"; n := (BatRef.get n) + 3; stmt' n s1; n := (BatRef.get n) - 3; doind n;
                            !"else\n"; n := (BatRef.get n) + 3; stmt' n s2; n := (BatRef.get n) - 3; doind n; !"fi\n"
        | FCall (name, args) -> doind n; expr @@ Language.Expr.FCall(name, args); !"\n"
        | Return e -> doind n; !"Return "; expr e; !"\n"
      in
      let indent = ref 0 in
      stmt' indent st
    let prog (fdefs, main) = stmt main
    let ints l = List.iter (fun i -> Printf.printf "%d\n" i) l
    let instr (instr : StackMachine.Instrs.t) =
      match instr with
      | S_PUSH n -> Printf.sprintf "S_PUSH %d" n
      | S_LD s -> "S_LD " ^ s
      | S_ST s -> "S_ST " ^ s
      | S_BINOP s -> "S_BINOP " ^ s
      | S_LBL s -> "S_LBL " ^ s
      | S_JMP s -> "S_JMP " ^ s
      | S_CJMP (s1, s2) -> "S_CJMP " ^ s1 ^ " " ^ s2
      | S_CALL (name, args) -> "S_CALL " ^ (String.concat " " (name::args))
      | S_RET -> "S_RET"
      | S_END -> "S_END"
    let instrs l = List.iter (fun i -> Printf.printf "%s\n" (instr i)) l
  end*)

(*module V = Language.Value
module E = Language.Expr
module S = Language.Stmt
let (!) s = Printf.printf "%s" s
let rec expr =
  function
  | E.Const n ->
     (let rec asd n = match n with
        | V.Int i -> !(Printf.sprintf "%d" i)
        | V.String s -> !s
        | V.Array a -> !"["; (List.iter (fun e -> asd e; !",") @@ Array.to_list a); !"]"
      in asd n)
  | E.Var x -> !x
  | E.Binop (o, l, r) -> !"("; expr l; !(" " ^ o ^ " "); expr r; !")"
  | E.FCall (name, args) -> !name; !"("; List.iter (fun arg -> expr arg; !",") args; !")"
  | E.UArray a -> !"["; (List.iter (fun e -> expr e; !"][") a); !"]"
  | E.BArray a -> !"["; (List.iter (fun e -> expr e; !"][") a); !"]"
  | E.ArrInd (a, i) -> expr a; !"["; expr i; !"]"
let stmt st =
  let rec stmt' n =
    let doind n = for i = 1 to BatRef.get n do !" " done in
    function
    | S.Skip -> doind n; !"Skip "; !"\n"
    | S.Seq (l, r) -> stmt' n l; stmt' n r
    | S.Assign (x, e) -> doind n; !x; !" := "; expr e; !"\n"
    | S.While (e, s) -> doind n; !"While "; expr e; !" do"; !"\n"; n := (BatRef.get n) + 3; stmt' n s; n := (BatRef.get n) - 3; doind n; !"od"; !"\n"
    | S.If (e, s1, s2) -> doind n; !"If "; expr e; !" then\n"; n := (BatRef.get n) + 3; stmt' n s1; n := (BatRef.get n) - 3; doind n;
                        !"else\n"; n := (BatRef.get n) + 3; stmt' n s2; n := (BatRef.get n) - 3; doind n; !"fi\n"
    | S.FCall (name, args) -> doind n; expr @@ E.FCall(name, args); !"\n"
    | S.Return e -> doind n; !"Return "; expr e; !"\n"
    | S.ArrAssign (name, inds, e) -> !name; !"["; (List.iter (fun e -> expr e; !"][") inds); !"]"; !" := "; expr e; !"\n"
  in
  let indent = ref 0 in
  stmt' indent st
let prog (fdefs, main) = List.iter (fun (_, _, main) -> stmt main) fdefs; stmt main*)
