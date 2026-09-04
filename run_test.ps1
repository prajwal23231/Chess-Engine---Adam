# ===========================================================
# ADAM Fastchess Automated Testing Runner (PowerShell)
# ===========================================================

param (
    [string]$Mode = "menu"  # "quick", "standard", "sprt", "update-base", "menu"
)

$ScriptDir   = $PSScriptRoot
$Fastchess   = "C:\Users\sriva\OneDrive\Desktop\fastchess-windows-x86-64\fastchess.exe"
$Book        = "C:\Users\sriva\OneDrive\Desktop\fastchess-windows-x86-64\app\tests\data\openings.epd"
$DevEngine   = Join-Path $ScriptDir "ADAM.exe"
$BaseEngine  = Join-Path $ScriptDir "ADAM_base.exe"
$PgnOut      = Join-Path $ScriptDir "test_results.pgn"
$Concurrency = 6

function Ensure-Baseline {
    if (-not (Test-Path $BaseEngine)) {
        Write-Host "`n[WARNING] ADAM_base.exe not found! Creating baseline from current ADAM.exe..." -ForegroundColor Yellow
        Copy-Item -Path $DevEngine -Destination $BaseEngine
        Write-Host "[SUCCESS] ADAM_base.exe created. Future modifications can now be tested against it.`n" -ForegroundColor Green
    }
}

function Run-Match ($TimeCtrl, $Rounds, $ExtraArgs = @()) {
    Ensure-Baseline
    $cmdArgs = @(
        "-engine", "cmd=$DevEngine", "name=ADAM_dev", "option.Hash=32",
        "-engine", "cmd=$BaseEngine", "name=ADAM_base", "option.Hash=32",
        "-each", "tc=$TimeCtrl",
        "-rounds", $Rounds, "-repeat",
        "-concurrency", $Concurrency,
        "-openings", "file=$Book", "format=epd", "order=random",
        "-draw", "movenumber=35", "movecount=8", "score=10",
        "-resign", "movecount=4", "score=600",
        "-pgnout", "file=$PgnOut"
    ) + $ExtraArgs

    Write-Host "`nLaunching Fastchess with $Concurrency concurrent games..." -ForegroundColor Cyan
    & $Fastchess $cmdArgs
}

switch ($Mode.ToLower()) {
    "quick" {
        Write-Host "Starting Quick Sanity Test (20 rounds, tc=5+0.05)..." -ForegroundColor Green
        Run-Match -TimeCtrl "5+0.05" -Rounds 20
    }
    "standard" {
        Write-Host "Starting Standard Match (100 rounds, tc=10+0.1)..." -ForegroundColor Green
        Run-Match -TimeCtrl "10+0.1" -Rounds 100
    }
    "sprt" {
        Write-Host "Starting SPRT Test (elo0=0, elo1=5, tc=8+0.08)..." -ForegroundColor Green
        Run-Match -TimeCtrl "8+0.08" -Rounds 2000 -ExtraArgs @("-sprt", "elo0=0", "elo1=5", "alpha=0.05", "beta=0.05")
    }
    "update-base" {
        Copy-Item -Path $DevEngine -Destination $BaseEngine -Force
        Write-Host "`n[SUCCESS] Current ADAM.exe saved as new ADAM_base.exe!`n" -ForegroundColor Green
    }
    default {
        Write-Host "===================================================" -ForegroundColor Cyan
        Write-Host "         ADAM CHESS ENGINE - TEST RUNNER           " -ForegroundColor Cyan
        Write-Host "===================================================" -ForegroundColor Cyan
        Write-Host "[1] Quick Sanity Test  (20 rounds / 40 games, tc=5+0.05, ~1-2 min)"
        Write-Host "[2] Standard Match     (100 rounds / 200 games, tc=10+0.1, ~10-15 min)"
        Write-Host "[3] SPRT Test          (elo0=0 elo1=5, auto-stops when +5 Elo is proven)"
        Write-Host "[4] Save current ADAM.exe as new ADAM_base.exe"
        Write-Host "[5] Exit`n"

        $choice = Read-Host "Select option [1-5]"
        switch ($choice) {
            "1" { Run-Match -TimeCtrl "5+0.05" -Rounds 20 }
            "2" { Run-Match -TimeCtrl "10+0.1" -Rounds 100 }
            "3" { Run-Match -TimeCtrl "8+0.08" -Rounds 2000 -ExtraArgs @("-sprt", "elo0=0", "elo1=5", "alpha=0.05", "beta=0.05") }
            "4" {
                Copy-Item -Path $DevEngine -Destination $BaseEngine -Force
                Write-Host "`n[SUCCESS] Current ADAM.exe saved as new ADAM_base.exe!`n" -ForegroundColor Green
            }
            default { Write-Host "Exiting." }
        }
    }
}
