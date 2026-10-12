import { clsx, type ClassValue } from 'clsx';
import { twMerge } from 'tailwind-merge';
export function cn(...inputs: ClassValue[]) { return twMerge(clsx(inputs)); }
export const base = import.meta.env.BASE_URL;
export function asset(path: string) { return base + path.replace(/^\//, ''); }
export function dateLabel(date: string) { return date.replaceAll('-', '.'); }
export function cleanTitle(title: string) { return title.replace(/^[\p{Extended_Pictographic}\uFE0F\s]+/u, '').trim(); }
