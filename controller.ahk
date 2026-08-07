#Requires AutoHotkey v2.0
#SingleInstance Force

Port := "COM6" 
ComPort := FileOpen(Port, "rw")

MatrixMultiply(M1, M2) {
    rowsM1 := M1.Length
    colsM1 := M1[1].Length
    colsM2 := M2.Length

    ; Initialize the result matrix with zeros
    Result := []
    loop rowsM1 {
        r := A_Index
        Result.Push([])
        loop colsM2 {
            Result[r].Push(0)
        }
    }

    ; Calculate dot products
    loop rowsM1 {
        i := A_Index
        loop colsM2 {
            j := A_Index
            loop colsM1 {
                k := A_Index
                Result[i][j] += M1[i][k] * M2[k]
            }
        }
    }
    return Result
}

screen := Gui("+AlwaysOnTop", "Controller Output")
screen.OnEvent("Close", (*) => ExitApp())
screen.SetFont("s10")
goal := screen.AddListView("w400 h200 Count2 Grid NoSort NoSortHdr", ["Segment", "X", "Y", "Z"])
goal.Add("", "Top", "0", "0", "0")
goal.ModifyCol(2, 100)
goal.ModifyCol(3, 100)
goal.ModifyCol(4, 100)
goal.Add("", "Bottom", "0", "0", "0")
output := screen.AddListView("w400 h200 Count2 Grid NoSort NoSortHdr", ["Component", "Target (kPa)", "Actual (kPa)", "Signal (PWM %)"])
output.Add("", "Tube 1", "0", "0", "0")
output.Add("", "Tube 2", "0", "0", "0")
output.Add("", "Tube 3", "0", "0", "0")
output.Add("", "Tube 4", "0", "0", "0")
output.Add("", "Tube 5", "0", "0", "0")
output.Add("", "Tube 6", "0", "0", "0")
screen.Show("Center AutoSize")

control := [
            [
                [1, 0, 0],
                [0, 1, 0],
                [0, 0, 1]
            ],
            [
                [1, 0, 0],
                [0, 1, 0],
                [0, 0, 1]
            ]
        ]

pos := [[0, 0, 0], [0, 0, 0]]

SetTimer(send_command, 100)
OnExit((*) => ComPort.Close())

send_command() {
    force := [[0, 0, 0], [0, 0, 0]]
    ; ToolTip("Controller: " . GetKeyState("1JoyName") . "`nButtons: " . GetKeyState("1JoyButtons") . "`nAxes: " . GetKeyState("1JoyInfo") . "`nX: " . Round(GetKeyState("1JoyX")) . ", Y: " . Round(GetKeyState("1JoyY")) . ", Z: " . Round(GetKeyState("1JoyZ")) . ", R: " . Round(GetKeyState("1JoyR")))
    force[1][1] := (Round(GetKeyState("1JoyX")) - 50) / 50
    force[1][2] := ((Round(GetKeyState("1JoyY"))) - 50) / 50
    if (GetKeyState("1Joy5")) {
        force[1][3] := 1
    } else if (GetKeyState("1Joy7")) {
        force[1][3] := -1
    }
    force[2][1] := (Round(GetKeyState("1JoyZ")) - 50) / 50
    force[2][2] := (Round(GetKeyState("1JoyR")) - 50) / 50
    if (GetKeyState("1Joy6")) {
        force[2][3] := 1
    } else if (GetKeyState("1Joy8")) {
        force[2][3] := -1
    }

    static update := 0
    update++
    for i, segment in force {
        for j, component in segment {
            if (Abs(component) < 0.1) {
                force[i][j] := 0
            }
            ; MsgBox(component)
            pos[i][j] += force[i][j]
        }
        if (Mod(update, 1) == 0) {
            goal.Modify(i, "Col2", pos[i]*)
            ; ToolTip("Part " . i . ": X=" . Round(pos[i][1], -1) . ", Y=" . Round(pos[i][2], -1) . ", Z=" . Round(pos[i][3], -1), 100, 100 + i*100, i*2+1)
            ; ToolTip("Force " . i . ": X=" . force[i][1] . ", Y=" . force[i][2] . ", Z=" . force[i][3], 100, 150 + i*100, i*2+2)
        }
    }

    command := []
    loop pos.Length {
        command.Push(MatrixMultiply(control[A_Index], pos[A_Index])*)
    }

    cmd := ""
    for i, segment in command {
        cmd .= i-1 . " " . Round(segment[1], -1) . " "
    }
    ComPort.WriteLine(cmd)
    receive := ComPort.ReadLine()
    receive := StrSplit(receive, "|")
    for i, segment in receive {
        if (i = 1) {
            continue
        }
        segment := StrSplit(segment, ",")
        if (Mod(update, 1) == 0) {
            output.Modify(i-1, "Col2", command[i-1][1], segment*)
            ; ToolTip("Segment " . i . ": goal=" . Round(command[i][1], -1) . ", set=" . Round(segment[1], -1) . ", PWM=" . Round(segment[2], -1), 100, 300 + i*100, i+2)
        }
    }
}