use std::fs::File;
use std::io;
use std::path::Path;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;
use std::time::Instant;
use memmap2::Mmap;
use rayon::prelude::*;

const CHUNK_SIZE: usize = 1 << 20; // 1MB chunks

fn mmap_file(path: &Path) -> io::Result<Mmap> {
    let file = File::open(path)?;
    unsafe { Mmap::map(&file) }
}

fn find_diffs(data1: &[u8], data2: &[u8], start: usize, end: usize, total_diffs: &AtomicUsize) -> Vec<(usize, usize)> {
    let mut diffs = Vec::new();
    let mut line = data1[..start].iter().filter(|&&b| b == b'\n').count() + 1;
    let mut pos = start;
    let len = std::cmp::min(data1.len(), data2.len());
    let end = std::cmp::min(end, len);

    while pos < end {
        if data1[pos] != data2[pos] {
            diffs.push((line, pos));
            total_diffs.fetch_add(1, Ordering::Relaxed);
            while pos < end && data1[pos] != b'\n' && data2[pos] != b'\n' {
                pos += 1;
            }
        } else if data1[pos] == b'\n' {
            line += 1;
        }
        pos += 1;
    }

    // If files have different lengths, count the remaining content as differences
    if data1.len() != data2.len() {
        let remaining_diffs = data1.len().abs_diff(data2.len());
        total_diffs.fetch_add(remaining_diffs, Ordering::Relaxed);
    }

    diffs
}

fn print_diffs(data1: &[u8], data2: &[u8], diffs: &[(usize, usize)]) {
    for &(line, pos) in diffs {
        println!("Diff at line {}, position {}:", line, pos);
        let start = data1[..pos].iter().rposition(|&b| b == b'\n').map_or(0, |i| i + 1);
        let end = data1[pos..].iter().position(|&b| b == b'\n').map_or(data1.len(), |i| pos + i);
        println!("File 1: {}", String::from_utf8_lossy(&data1[start..end]));
        println!("File 2: {}", String::from_utf8_lossy(&data2[start..end]));
        println!();
    }
}

fn compare_files(path1: &Path, path2: &Path) -> io::Result<usize> {
    let start_time = Instant::now();
    let data1 = mmap_file(path1)?;
    let data2 = mmap_file(path2)?;
    let total_diffs = Arc::new(AtomicUsize::new(0));
    let chunk_size = std::cmp::max(CHUNK_SIZE, data1.len() / rayon::current_num_threads());
    let max_len = std::cmp::max(data1.len(), data2.len());

    let diffs: Vec<Vec<(usize, usize)>> = (0..max_len)
        .into_par_iter()
        .step_by(chunk_size)
        .map(|start| {
            let end = std::cmp::min(start + chunk_size, max_len);
            find_diffs(&data1, &data2, start, end, &total_diffs)
        })
        .collect();

    let total_diffs = total_diffs.load(Ordering::Relaxed);
    if total_diffs == 0 {
        println!("Files are identical.");
    } else {
        println!("Total differences: {}", total_diffs);
        for chunk_diffs in diffs {
            print_diffs(&data1, &data2, &chunk_diffs);
        }
    }

    let duration = start_time.elapsed();
    println!("Comparison completed in {:.6} seconds", duration.as_secs_f64());
    Ok(total_diffs)
}

fn main() -> io::Result<()> {
    let args: Vec<String> = std::env::args().collect();
    if args.len() != 3 {
        eprintln!("Usage: {} <file1> <file2>", args[0]);
        std::process::exit(1);
    }
    let path1 = Path::new(&args[1]);
    let path2 = Path::new(&args[2]);
    compare_files(path1, path2)?;
    Ok(())
}
